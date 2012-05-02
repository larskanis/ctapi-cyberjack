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



#import "PinDialog.h"
#import "../fxcyberjack/icons/icons.cpp"


#define I18N(msg) msg



@implementation PinDialog


- (id) initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)aStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)flag {
  self=[super initWithContentRect:contentRect styleMask:aStyle backing:bufferingType defer:flag];

  if (self!=nil) {
    _rsctLabel = [[NSTextField alloc] init];
    [_rsctLabel setStringValue:@"Reiner SCT cyberJack"];
    [_rsctLabel setBezeled:NO];
    [_rsctLabel setEditable:NO];
    [_rsctLabel setSelectable:NO];
    [_rsctLabel setDrawsBackground:NO];
    [[self contentView] addSubview:_rsctLabel];
  
    _stageLabel = [[NSTextField alloc] init];
    [_stageLabel setStringValue:@"1/2"];
    [_stageLabel setBezeled:NO];
    [_stageLabel setEditable:NO];
    [_stageLabel setSelectable:NO];
    [_stageLabel setDrawsBackground:NO];
    [_stageLabel setAlignment:NSRightTextAlignment];
    [[self contentView] addSubview:_stageLabel];
  
    _hLine = [[NSBox alloc] init];
    [_hLine setBoxType:NSBoxSeparator];
    [[self contentView] addSubview:_hLine];
  
    _imageData=[[NSData alloc] initWithBytes: fxcj_icon_cy_ecom_b length:sizeof(fxcj_icon_cy_ecom_b)];
    _image = [[NSImage alloc] initWithData: _imageData];
    _imageView = [[NSImageView alloc] init];
    [_imageView setImage:_image];
    [_imageView setBounds: NSMakeRect (0, 0, 200, 200)];
    [[self contentView] addSubview:_imageView];
  
    _textLabel = [[NSTextField alloc] init];
    [_textLabel setStringValue:@"Bitte geben Sie die Pin ein:"];
    [_textLabel setBezeled:NO];
    [_textLabel setEditable:NO];
    [_textLabel setSelectable:NO];
    [_textLabel setDrawsBackground:NO];
    [_textLabel setAlignment:NSCenterTextAlignment];
    [[self contentView] addSubview:_textLabel];
  
    _entryLabel = [[NSTextField alloc] init];
    [_entryLabel setStringValue:@"****"];
    [_entryLabel setBezeled:NO];
    [_entryLabel setEditable:NO];
    [_entryLabel setSelectable:NO];
    [_entryLabel setDrawsBackground:NO];
    [_entryLabel setAlignment:NSCenterTextAlignment];
    [[self contentView] addSubview:_entryLabel];
  
    _progressWidget=[[NSProgressIndicator alloc] init];
    [_progressWidget setIndeterminate: FALSE];
    [_progressWidget setMinValue: 0.0];
    [_progressWidget setMaxValue: 100.0];
    [_progressWidget setDoubleValue: 100.0];
    [[self contentView] addSubview:_progressWidget];
  }

  [[NSNotificationCenter defaultCenter] addObserver:self
     selector:@selector(windowDidResize:)
     name:NSWindowDidResizeNotification
     object:nil];

  [[NSNotificationCenter defaultCenter] addObserver:self
     selector:@selector(windowWillClose:)
     name:NSWindowWillCloseNotification
     object:nil];

  return self;
}



- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  if (_rsctLabel!=nil) [_rsctLabel release];
  if (_stageLabel!=nil) [_stageLabel release];
  if (_hLine!=nil) [_hLine release];
  if (_imageView!=nil) [_imageView release];
  if (_image!=nil) [_image release];
  if (_imageData!=nil) [_imageData release];
  if (_textLabel!=nil) [_textLabel release];
  if (_entryLabel!=nil) [_entryLabel release];
  if (_progressWidget!=nil) [_progressWidget release];

  [super dealloc];
}



-(void) layout {
  NSRect r=[self frame];
  int w;
  int h;
  int x;
  int y;

  w=r.size.width;
  h=r.size.height;
  x=10;
  y=h-20;

  [_rsctLabel setFrame: NSMakeRect(x, y-20, ((w-20)/2), 20)];
  x=((w-20)/2)+10;
  [_stageLabel setFrame: NSMakeRect(x, y-20, ((w-20)/2), 20)];
  y-=20;
  x=10;
  [_hLine setFrame: NSMakeRect(x, y-5, w-(x*2), 5)];
  y-=10;
  [_imageView setFrame: NSMakeRect(x, y-200, 200, 200)];
  [_imageView setBounds: NSMakeRect (0, 0, 200, 200)];
  x+=200;
  [_textLabel setFrame: NSMakeRect(x, y-60, (w-x-10), 40)];
  [_entryLabel setFrame: NSMakeRect(x, y-100, (w-x-10), 20)];
  y-=210;
  x=10;
  [_progressWidget setFrame: NSMakeRect(x, y-20, (w-x*2), 20)];

}



- (void) windowDidResize:(NSNotification*) notification {
  [self layout];

}



- (void) windowWillClose:(NSNotification*) notification {
  fprintf(stderr, "Window will close.\n");
}



- (void) setCurrentStage:(int) stage of:(int) stages withText:(NSString*) text {
  char numbuf[32];

  snprintf(numbuf, sizeof(numbuf)-1, "%d/%d", stage+1, stages);
  NSString *str=[NSString stringWithUTF8String: numbuf];
  [_stageLabel setStringValue: str];
  //[str release];

  [_textLabel setStringValue: text];
}



- (void) setNumDigits:(int)num {
  int i;
  char buf[64];

  if (num>=(sizeof(buf)-1))
    num=sizeof(buf)-1;
  for (i=0; i<num; i++)
    buf[i]='*';
  buf[i]=0;

  NSString *str=[NSString stringWithUTF8String: buf];
  [_entryLabel setStringValue: str];
  //[str release];
}



- (void) setTimeRemaining:(double) secs of:(double)total {
  [_progressWidget setMinValue: 0.0];
  [_progressWidget setMaxValue: total];
  [_progressWidget setDoubleValue: secs];
}





@end


