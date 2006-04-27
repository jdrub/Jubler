/*
 * JSubPreviewDialog.java
 *
 * Created on 21 Σεπτέμβριος 2005, 10:26 πμ
 *
 * This file is part of Jubler.
 *
 * Jubler is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
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

package com.panayotis.jubler.preview;

import static com.panayotis.jubler.i18n.I18N._;

import com.panayotis.jubler.Jubler;
import com.panayotis.jubler.subs.JSubEditor;
import java.awt.BorderLayout;
import java.io.File;

/**
 *
 * @author  teras
 */
public class JSubPreviewDialog extends javax.swing.JDialog {
    JSubPreview preview;
    
    /** Creates new form JSubPreviewDialog */
    public JSubPreviewDialog(Jubler parent, JSubPreview preview) {
        super(parent, false);
        initComponents();
        
        this.preview = preview;
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {

        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setTitle(_("Subtitles preview"));
        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(java.awt.event.WindowEvent evt) {
                formWindowClosing(evt);
            }
        });

        pack();
    }
    // </editor-fold>//GEN-END:initComponents
    
    private void formWindowClosing(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_formWindowClosing
        preview.dialogClosed();
    }//GEN-LAST:event_formWindowClosing
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    // End of variables declaration//GEN-END:variables
    
}
