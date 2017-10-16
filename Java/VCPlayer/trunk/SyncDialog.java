/*
 * SyncDialog.java
 *
 * Created on August 19, 2003, 1:18 PM
 */

package VCPlayer;

public class SyncDialog extends javax.swing.JDialog {
    
    private boolean shouldSync = false;
    private int window = 500;
    
    public SyncDialog(java.awt.Frame parent, String title, boolean modal) {
        super(parent, title, modal);
        initComponents();
        buttonGroup1.add(noSync);
        buttonGroup1.add(regSync);
        buttonGroup1.add(lagSync);
        noSync.setSelected(true);
        setSize(400,300);
    }
    
    public int getWindow() {
        return window;
    }
    
    public boolean getSync() {
        return shouldSync;
    }
    

    private void initComponents() {//GEN-BEGIN:initComponents
        buttonGroup1 = new javax.swing.ButtonGroup();
        noSync = new javax.swing.JRadioButton();
        regSync = new javax.swing.JRadioButton();
        lagSync = new javax.swing.JRadioButton();
        lagTime = new javax.swing.JTextField();
        seconds = new javax.swing.JLabel();
        okayButton = new javax.swing.JButton();

        getContentPane().setLayout(null);

        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(java.awt.event.WindowEvent evt) {
                closeDialog(evt);
            }
        });

        noSync.setText("Do Not Sync Timelines");
        getContentPane().add(noSync);
        noSync.setBounds(30, 10, 160, 30);

        regSync.setText("Click Here If Both Should Use The Same Timeline");
        getContentPane().add(regSync);
        regSync.setBounds(30, 50, 300, 24);

        lagSync.setText("Sync Timelines With This Given Lag-Time");
        getContentPane().add(lagSync);
        lagSync.setBounds(30, 90, 257, 24);

        lagTime.setText("0.5");
        getContentPane().add(lagTime);
        lagTime.setBounds(80, 130, 30, 20);

        seconds.setText("Seconds");
        getContentPane().add(seconds);
        seconds.setBounds(120, 130, 60, 16);

        okayButton.setText("Okay");
        okayButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                okayButtonActionPerformed(evt);
            }
        });

        getContentPane().add(okayButton);
        okayButton.setBounds(150, 200, 63, 26);

        pack();
    }//GEN-END:initComponents

    private void okayButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_okayButtonActionPerformed
        if(regSync.isSelected() || lagSync.isSelected())
            shouldSync = true;
        else
            shouldSync = false;
        try {
            double temp = Double.parseDouble(lagTime.getText());
            temp = temp*1000;
            window = (int)temp;
        }
        catch (NumberFormatException e) {
            System.out.println("Lag Time was not a number");
        }
        closeDialog(new java.awt.event.WindowEvent(this,java.awt.event.WindowEvent.WINDOW_CLOSING));
    }//GEN-LAST:event_okayButtonActionPerformed
    
    /** Closes the dialog */
    private void closeDialog(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_closeDialog
        setVisible(false);
        dispose();
    }//GEN-LAST:event_closeDialog
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        new SyncDialog(new javax.swing.JFrame(),"No Title", true).show();
    }
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JRadioButton lagSync;
    private javax.swing.JRadioButton noSync;
    private javax.swing.JRadioButton regSync;
    private javax.swing.JLabel seconds;
    private javax.swing.ButtonGroup buttonGroup1;
    private javax.swing.JButton okayButton;
    private javax.swing.JTextField lagTime;
    // End of variables declaration//GEN-END:variables
    
}
