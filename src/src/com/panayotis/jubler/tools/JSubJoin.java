/*
 * JSubJoin.java
 *
 * Created on 25 Ιούνιος 2005, 3:31 μμ
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

package com.panayotis.jubler.tools;

import com.panayotis.jubler.Jubler;
import com.panayotis.jubler.time.Time;
import com.panayotis.jubler.time.gui.JTimeSpinner;
import java.awt.BorderLayout;
import java.util.Vector;

import static com.panayotis.jubler.i18n.I18N._;

/**
 *
 * @author  teras
 */
public class JSubJoin extends javax.swing.JPanel {
    private JTimeSpinner joinpos;
    private Vector<Jubler> privlist;
    
    /** Creates new form JSplit */
    public JSubJoin(Vector<Jubler> list, Jubler current) {
        joinpos = new JTimeSpinner();
        privlist = new Vector<Jubler>();
        
        initComponents();
        TShift.add(joinpos, BorderLayout.CENTER);
        
        for ( int i = 0 ; i <list.size() ; i++) {
            if ( list.elementAt(i) != current ) {
                SubWindow.addItem(list.elementAt(i).getFileName());
                privlist.add(list.elementAt(i));
            }
        }
        joinpos.setToolTipText(_("Use the selected amount of time as space between the two subtitles"));
    }
    
    public boolean isPrepend() {
        return RPrepend.isSelected();
    }
    
    public Jubler getOtherSubs() {
        return privlist.elementAt(SubWindow.getSelectedIndex());
    }
    
    public Time getGap() {
        return (Time)(joinpos.getModel().getValue());
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        Position = new javax.swing.ButtonGroup();
        jPanel1 = new javax.swing.JPanel();
        SubWindow = new javax.swing.JComboBox();
        RPrepend = new javax.swing.JRadioButton();
        RAppend = new javax.swing.JRadioButton();
        TShift = new javax.swing.JPanel();

        setLayout(new java.awt.BorderLayout());

        jPanel1.setLayout(new java.awt.GridLayout(0, 1));

        jPanel1.setBorder(new javax.swing.border.TitledBorder(_("Use the following subtitles")));
        SubWindow.setToolTipText(_("Subtitles file to use"));
        jPanel1.add(SubWindow);

        Position.add(RPrepend);
        RPrepend.setText(_("Prepend subtitles"));
        RPrepend.setToolTipText(_("Put subtitles in the beginning of the current subtitles"));
        jPanel1.add(RPrepend);

        Position.add(RAppend);
        RAppend.setSelected(true);
        RAppend.setText(_("Append Subtitles"));
        RAppend.setToolTipText(_("Put subtitles at the end of the current subtitles"));
        jPanel1.add(RAppend);

        add(jPanel1, java.awt.BorderLayout.CENTER);

        TShift.setLayout(new java.awt.BorderLayout());

        TShift.setBorder(new javax.swing.border.TitledBorder(_("Leave gap")));
        add(TShift, java.awt.BorderLayout.SOUTH);

    }
    // </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.ButtonGroup Position;
    private javax.swing.JRadioButton RAppend;
    private javax.swing.JRadioButton RPrepend;
    private javax.swing.JComboBox SubWindow;
    private javax.swing.JPanel TShift;
    private javax.swing.JPanel jPanel1;
    // End of variables declaration//GEN-END:variables
    
}
