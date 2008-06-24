// Bare-bones ffmpeg (cvs latest) precise time seeking and audio decoding
//
// Precise time seeking and audio decoding code: Thanos Kyritsis
// Date: November 2005
//
// JNI, Java specific code: Panayotis Katsaloulis
// Date: November 2005
//

/* This file is part of Jubler.
 *
 * Jubler is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Jubler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Jubler; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "libavformat/avformat.h"
#include <stdlib.h>
#include <stdio.h>

#include "com_panayotis_jubler_media_preview_decoders_FFMPEG.h"
#include "utilities.h"

jboolean decodeAudio(JNIEnv* env, jobject this, const char *input_filename, const char *output_filename, jlong seek_time_start, jlong seek_time_stop);
AVStream *add_audio_stream(JNIEnv* env, jobject this, AVFormatContext *oc, int codec_id, int sample_rate, int channels); void fput_le32(FILE *stream, long int val);
size_t audio_out(char *, size_t, size_t, FILE *);


JNIEXPORT jboolean JNICALL Java_com_panayotis_jubler_media_preview_decoders_FFMPEG_createClip(JNIEnv * env, jobject this, jstring audio, jstring wav, jlong start, jlong stop) {
    const char * audio_c;
    const char * wav_c;
    jboolean ret = JNI_FALSE;
    
    /* translate Java strings into C strings */
    audio_c = (*env)->GetStringUTFChars(env, audio, 0);
    wav_c = (*env)->GetStringUTFChars(env, wav, 0);
 
    ret = decodeAudio(env, this, audio_c, wav_c, start, stop);
    
    /* free memory reserved for Java->C strings */
    (*env)->ReleaseStringUTFChars(env, audio, audio_c);
    (*env)->ReleaseStringUTFChars(env, wav, wav_c);
    
    return ret;
}



jboolean decodeAudio(JNIEnv * env, jobject this, const char *input_filename, const char *output_filename, jlong seek_time_start, jlong seek_time_stop) {
    AVCodec *codec=NULL, *vcodec=NULL;
    AVPacket pkt;
    AVCodecContext *ccx=NULL;
    AVFormatContext *fcx=NULL, *ofcx=NULL;
    AVOutputFormat *fmt=NULL;
    AVStream *audio_st=NULL;
    int got_audio, len, err=0, audio_index=-1, i=0, pack_duration=0, packsize=0, codec_is_open=-1, video_index=-1;
    long int file_size=0, header_size=0;
    jlong pack_pts=0;
    FILE *outfile=NULL;
    char *outbuf=NULL;
    unsigned char *packptr;
    jboolean ret = JNI_TRUE, nobrk = JNI_TRUE;
    
    av_register_all();
    
    /* Open the input/output files */
    err = av_open_input_file(&fcx, input_filename, NULL, 0, NULL);
    if(err<0){
        DEBUG(env, this, "decodeAudio", "Could not open file '%s'.", input_filename);
        ret = JNI_FALSE;
    }
    outbuf = malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE);
    if(outbuf==NULL) {
        DEBUG(env, this, "decodeAudio", "Could not allocate memory for outbuf.");
        ret = JNI_FALSE;
    }
    if (ret != JNI_FALSE) {
        /* Find the stream info */
        av_find_stream_info(fcx);
        
        /* Find the first supported codec in the audio streams */
        for(i=0; i<fcx->nb_streams; i++){
            ccx=fcx->streams[i]->codec;
            if(ccx->codec_type==CODEC_TYPE_AUDIO) {
                /* Found an audio stream, check if codec is supported */
                codec = avcodec_find_decoder(ccx->codec_id);
                if(codec){
                    /* codec is supported, proceed */
                    audio_index = i;
                    break;
                }
            }
        }
        /* Find the first supported video codec in the stream
         * This is only meant for seeking. When we have muxed
         * audio and video stream we have to seek by the video
         */
        for(i=0; i<fcx->nb_streams; i++) {
            if(fcx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
                /* Found a video stream, check if codec is supported */
                vcodec = avcodec_find_decoder(fcx->streams[i]->codec->codec_id);
                if(vcodec) {
                    /* codec is supported, proceed */
                    video_index = i;
                    break;
                }
            }
        }
        /* If we didn't find any supported video stream, it might mean
         * we are dealing with an audio only stream, so we can use the
         * audio_index for seeking */
        if (video_index < 0) video_index = audio_index;
    }
    
    /* Find codec id */
    if(audio_index < 0){
        DEBUG(env, this, "decodeAudio", "Audio stream with supported codec not found.");
        ret = JNI_FALSE;
    }
    else {
        /* open it */
        if ((codec_is_open = avcodec_open(ccx, codec)) < 0) {
            DEBUG(env, this, "decodeAudio", "Could not open codec.");
            ret = JNI_FALSE;
        }
        else {
            if (fcx->start_time != AV_NOPTS_VALUE) {
                seek_time_start += fcx->start_time;
                seek_time_stop += fcx->start_time;
            }
            /* Check that timepos_start is smaller than timepos_stop */
            if (seek_time_start < seek_time_stop) {
                /* Check that timepos is inside duration */
                if (seek_time_start <= fcx->duration && seek_time_stop <= fcx->duration) {
                    /* All ok */
                    /*  Seek to the nearest keyframe before the seek_time we asked */
                    /* Notice: always seek to video stream because if it is an avi
                     * we need to seek by the video stream. not the audio */
                    av_seek_frame(fcx, video_index, av_rescale_q(seek_time_start, AV_TIME_BASE_Q, fcx->streams[video_index]->time_base), AVSEEK_FLAG_BACKWARD);
                    /* Create WAV headers */
                    
                    /* allocate the output media context */
                    ofcx = av_alloc_format_context();
                    
                    if (!ofcx) {
                        DEBUG(env, this, "decodeAudio", "Memory error!");
                        ret = JNI_FALSE;
                    }
                    
                    /* use wav as the output format of the file */
                    fmt = guess_format("wav", NULL, NULL);
                    if (!fmt) {
                        DEBUG(env, this, "decodeAudio", "Could not find suitable output format.");
                        ret = JNI_FALSE;
                    }
                    
                    if(ret != JNI_FALSE) {
                        ofcx->oformat = fmt;
                        snprintf(ofcx->filename, sizeof(ofcx->filename), "%s", output_filename);
                        
                        /* add the audio stream using the default format codec and initialize the codec */
                        if (fmt->audio_codec != CODEC_ID_NONE) {
                            // stereo downmix
                            if (ccx->channels > 2) {
                                ccx->channels = 2;
                                ccx->request_channels = 2;
                            }
                            audio_st = add_audio_stream(env, this, ofcx, fmt->audio_codec, ccx->sample_rate, ccx->channels);
                        }
                        
                        /* set the output parameters (must be done even if no parameters) */
                        if (av_set_parameters(ofcx, NULL) < 0) {
                            DEBUG(env, this, "decodeAudio", "Invalid output format parameters.");
                            ret = JNI_FALSE;
                        }
                        else {
                            /* open the output file, if needed */
                            if (!(fmt->flags & AVFMT_NOFILE)) {
                                if (url_fopen(&ofcx->pb, output_filename, URL_WRONLY) < 0) {
                                    DEBUG(env, this, "decodeAudio", "Could not open file '%s'.", output_filename);
                                    ret = JNI_FALSE;
                                }
                            }
                            
                            /* write the stream header, if any */
                            if (av_write_header(ofcx) < 0) {
                                DEBUG(env, this, "decodeAudio", "Could not write header for output file.") ;
                                ret = JNI_FALSE;
                            }
                            else {
                                /* Clean up part 1*/
                                /* free the streams */
                                for(i = 0; i < ofcx->nb_streams; i++) {
                                    av_freep(&ofcx->streams[i]);
                                }
                            }
                        }
 
                    	/* close the output file */
                    	if (!(fmt->flags & AVFMT_NOFILE)) {
                        	url_fclose(ofcx->pb);
                    	}
                    }
                   	
					/* Clean up part 2*/
                    
					/* free the stream */
                    if(ofcx != NULL) av_free(ofcx);
                    
                    /* Create WAV headers end */
                    
                    if (ret != JNI_FALSE) {
                        outfile = fopen(output_filename, "r+b");
                        if (!outfile) {
                            DEBUG(env, this, "decodeAudio", "Could not open file '%s'", output_filename);
                            ret = JNI_FALSE;
                        }
                        else {
                            /* truncate the last four 0 bytes of the empty header */
                            fseek(outfile, -4, SEEK_END);
                            header_size = ftell(outfile);
                        }
                    }
                }
                else {
                    DEBUG(env, this, "decodeAudio", "Seek start/stop time cannot be greater than input file's duration.");
                    ret = JNI_FALSE;
                }
            }
            else {
                DEBUG(env, this, "decodeAudio", "Seek start time is not smaller than seek stop time.");
                ret = JNI_FALSE;
            }
        }
    }
    
    while(ret != JNI_FALSE && nobrk != JNI_FALSE) {
        /* Read a frame/packet */
        if(av_read_frame(fcx, &pkt) < 0 ) break;
        packsize = pkt.size;
        packptr = pkt.data;
        /* Make sure this packet belongs to the stream we want */
        if(pkt.stream_index==audio_index){
            while (packsize > 0 && nobrk != JNI_FALSE) {
                // Rescale the times
                pack_pts = av_rescale_q(pkt.pts, fcx->streams[audio_index]->time_base, AV_TIME_BASE_Q);
                pack_duration = av_rescale_q(pkt.duration, fcx->streams[audio_index]->time_base, AV_TIME_BASE_Q);
                /* Decode the paket */
                // stereo downmix
                if (ccx->channels > 2) {
                    ccx->channels = 2;
                    ccx->request_channels = 2;
                }
                got_audio = AVCODEC_MAX_AUDIO_FRAME_SIZE;
                len = avcodec_decode_audio2(ccx, (short *)outbuf, &got_audio, packptr, packsize);
                
                if (len < 0) {
                    DEBUG(env, this, "decodeAudio", "Error while decoding.");
                    continue;
                }
                
                packsize -= len;
                packptr += len;
                
                if (got_audio > 0) {
                    /* If we got the time exactly, or we are already past the seek time,
                     * this is the frame we want */
                    if (pack_pts >= seek_time_start) {
                        /* if a frame has been decoded, output it */
                        audio_out(outbuf, 1, got_audio, outfile);
                        /* if the next frame gets past our stop time, we want to stop decoding */
                        if ( pack_pts + pack_duration > seek_time_stop ) {
                            av_free_packet(&pkt);
                            nobrk = JNI_FALSE;
                            break;
                        }
                    }
                    /* If the next frame will be past our start seek time, this is the frame we want */
                    else if (pack_pts + pack_duration > seek_time_start) {
                        /* if a frame has been decoded, output it */
                        audio_out(outbuf, 1, got_audio, outfile);
                        /* if the next frame gets past our stop time, we want to stop decoding */
                        if ( pack_pts + pack_duration > seek_time_stop ) {
                            av_free_packet(&pkt);
                            nobrk = JNI_FALSE;
                            break;
                        }
                    }
                }
            }
        }
        av_free_packet(&pkt);
    }
    
    /* Write trailer */
    if (ret != JNI_FALSE ) {
        file_size = ftell(outfile);
        fseek(outfile, header_size, SEEK_SET);
        fput_le32(outfile, file_size - header_size - sizeof(file_size));
    }
    
    /* Clean up */
    if(outfile != NULL)    fclose(outfile);
    if(codec_is_open >= 0) avcodec_close(ccx);
    if(outbuf != NULL)     free(outbuf);
    if(fcx != NULL)        av_close_input_file(fcx);
    
    return ret;
}


AVStream *add_audio_stream(JNIEnv * env, jobject this, AVFormatContext *oc, int codec_id, int sample_rate, int channels) {
    AVCodecContext *c;
    AVStream *st;
    jboolean retflag = JNI_TRUE;
    
    st = av_new_stream(oc, 1);
    if (!st) {
        DEBUG(env, this, "add_audio_stream", "Could not allocate stream.");
        retflag = JNI_FALSE;
    }
    
    if (retflag != JNI_FALSE) {
        c = st->codec;
        c->codec_id = codec_id;
        c->codec_type = CODEC_TYPE_AUDIO;
        
        /* put sample parameters */
        //c->bit_rate = 64000;
        c->sample_rate = sample_rate;
        c->channels = channels;
    }
    
    if(retflag != JNI_FALSE)
        return st;
    else
        return NULL;
}

void fput_le32(FILE *stream, long int val) {
    fputc(val, stream);
    fputc(val >> 8, stream);
    fputc(val >> 16, stream);
    fputc(val >> 24, stream);
}

size_t audio_out(char *ptr, size_t size, size_t nmemb, FILE *stream) {
    char swap;
    int i=0;
    
    if (!isLittleEndian()) {
        for (i=0; i<= ((size*nmemb)-1); i++) {
            swap = ptr[i]; // high
            ptr[i] = ptr[i+1]; // low
            ptr[++i] = swap ; // putting high back
        }
        
    }
    
    return fwrite(ptr, size, nmemb, stream);
}

