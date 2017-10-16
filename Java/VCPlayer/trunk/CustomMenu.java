/*
 * CustomMenu.java
 *
 * This module is the popup TOC menu.  It is created by and included in the VCP 
 * module.  When it is created it takes the titles of the corresponding PowerPoint 
 * slides and creates a popup menu item for each one.  When a user clicks on an 
 * item, the CustomMenu pauses the presentation and sets each component of the 
 * presentation to the correct media time (in the case of a slide-based media 
 * the media time is used to calculate which slide should be displayed and for 
 * how long.  
 */

package VCPlayer;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;


public class CustomMenu extends javax.swing.JPopupMenu implements ActionListener{
    private Vector names, placeMarkers;
    private int totalSlides;
    private int pageNumber = 0;
    private int PAGE_SIZE = 18;
    private MouseListener theListener;
    private VCP parent;
    private int locX = 0, locY = 0;
    private Component tempComponent;
    private JMenuItem tempPrev;
    private JMenuItem tempNext;
    
    public CustomMenu(Vector nam, Vector dur, int total, VCP PT) {
        names = nam;
        totalSlides = total;
        parent = PT;
        placeMarkers = new Vector(100);
        int totalSoFar = 0;
        tempPrev = new JMenuItem("Previous...");
        tempNext = new JMenuItem("Next...");
        tempPrev.setEnabled(false);
        tempPrev.addActionListener(this);
        tempNext.addActionListener(this);
        if (totalSlides < PAGE_SIZE + 1) // By liwh: fix bug of less than Page_size slides. 09/22/04
            tempNext.setEnabled(false);
        add(tempPrev);
        for(int i = 0; i < totalSlides; i++) {
            placeMarkers.addElement(new Integer(totalSoFar));
            totalSoFar += ((Integer)dur.elementAt(i)).intValue();
            if (i < PAGE_SIZE)
            {
                JMenuItem temp = new JMenuItem((String)names.elementAt(i));
                if (((String)names.elementAt(i)).length() > VCP.MENU_WIDTH)
                    temp.setToolTipText((String)(parent.getSlideFullNames()).elementAt(i));
                temp.addActionListener(this);
                add(temp);
            }
        }
        add(tempNext);
        setLightWeightPopupEnabled(false);
        theListener = new PopupListener();
    }
    
    // By liwh: 9/26/04 
    public int getTimePosition(int place) {
        return ((Integer)placeMarkers.elementAt(place)).intValue();
    }
    
    public void actionPerformed(java.awt.event.ActionEvent actionEvent) {

        try {
            String tempStr = actionEvent.getActionCommand();
            if (tempStr == "Next...") {
                pageNumber ++;
                for(int i = 0; i < PAGE_SIZE; i++) {
                    if ((pageNumber * PAGE_SIZE + i) < totalSlides ){
                        ((JMenuItem)this.getComponent(i+1)).setText((String)names.elementAt(i + PAGE_SIZE*pageNumber));
                        if (((String)names.elementAt(i + PAGE_SIZE*pageNumber)).length() > VCP.MENU_WIDTH)
                            ((JMenuItem)this.getComponent(i+1)).setToolTipText((String)(parent.getSlideFullNames()).elementAt(i + PAGE_SIZE*pageNumber));
                        else 
                            ((JMenuItem)this.getComponent(i+1)).setToolTipText(null);
                        
                    }
                    else {
                        ((JMenuItem)this.getComponent(i+1)).setText(" ");
                        ((JMenuItem)this.getComponent(i+1)).setEnabled(false);
                    }
                }

                tempPrev.setEnabled(true);
                if (((pageNumber + 1) * PAGE_SIZE) >= totalSlides )
                    tempNext.setEnabled(false);
                show(tempComponent,locX,locY);
                
            }
            else if (tempStr == "Previous...") {
                pageNumber --;
                for(int i = 0; i < PAGE_SIZE; i++) {
                    ((JMenuItem)this.getComponent(i+1)).setEnabled(true);
                    ((JMenuItem)this.getComponent(i+1)).setText((String)names.elementAt(i + PAGE_SIZE*pageNumber));
                    if (((String)names.elementAt(i + PAGE_SIZE*pageNumber)).length() > VCP.MENU_WIDTH)
                        ((JMenuItem)this.getComponent(i+1)).setToolTipText((String)(parent.getSlideFullNames()).elementAt(i + PAGE_SIZE*pageNumber));
                    else 
                        ((JMenuItem)this.getComponent(i+1)).setToolTipText(null);
                }

                tempNext.setEnabled(true);
                if (pageNumber == 0)
                    tempPrev.setEnabled(false);
                show(tempComponent,locX,locY);
            }
            else {
                StringTokenizer temp = new StringTokenizer(actionEvent.getActionCommand());
                int slideClicked = Integer.parseInt(temp.nextToken()) - 1;
                parent.clearLoaded(slideClicked);
                parent.waitForCurCursorReady();
                parent.setPosition(((Integer)placeMarkers.elementAt(slideClicked)).intValue());
            }
        }
        catch (NumberFormatException e) {
            System.out.println("String Problem");
        }
    }
    
    public MouseListener getListener() {
        return theListener;
    }
    
    
    
    class PopupListener extends MouseAdapter {
        public void mousePressed(MouseEvent e) {
            maybeShowPopup(e);
        }

        public void mouseReleased(MouseEvent e) {
            maybeShowPopup(e);
        }

        private void maybeShowPopup(MouseEvent e) {
            if (e.isPopupTrigger()) {
                locX = e.getX();
                locY = e.getY();
                tempComponent = e.getComponent();
                show(e.getComponent(),locX, locY);
            }
        }
    }
}

