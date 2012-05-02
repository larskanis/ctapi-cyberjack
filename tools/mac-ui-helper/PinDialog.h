/***************************************************************************
    begin       : Sat Jul 04 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RSCT_MAC_PINDIALOG_H
#define RSCT_MAC_PINDIALOG_H


#import <Cocoa/Cocoa.h>

#include "base/message.h"


@interface PinDialog: NSWindow {
  NSTextField *_rsctLabel;
  NSTextField *_stageLabel;
  NSBox *_hLine;

  NSData *_imageData;
  NSImage *_image;
  NSImageView *_imageView;

  NSTextField *_textLabel;
  NSTextField *_entryLabel;
  NSProgressIndicator *_progressWidget;
}


- (id) initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag;

- (void)dealloc;

- (void) layout;
- (void) windowDidResize:(NSNotification*) notification;
- (void) windowWillClose:(NSNotification*) notification;

- (void) setCurrentStage:(int)stage of:(int) stages withText:(NSString*) text;
- (void) setTimeRemaining:(double) secs of:(double)total;

- (void) setNumDigits:(int) num;

@end



#endif

