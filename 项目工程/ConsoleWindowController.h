
#import <Cocoa/Cocoa.h>

@interface ConsoleWindowController : NSWindowController
{
    NSTextView *textView;
    IBOutlet NSButton *checkScroll;
    IBOutlet NSButton *topCheckBox;
    IBOutlet NSButton *exportLuaLog;
    NSMutableArray *linesCount;
    NSUInteger traceCount;
    //是否可以正常接收消息
    BOOL    _isReceiveMsgNormal;
    NSMutableArray<NSNumber*> *_reserveMutableArray;
    NSMutableArray<NSAttributedString*> *_reserveStringArray;
    NSInteger  _reserveCount;
    
    NSDictionary  *_fontAttrib;
}

@property (assign) IBOutlet NSTextView *textView;

- (void) trace:(NSString*)msg;
- (IBAction)onClear:(id)sender;
- (IBAction)onPause:(id)sender;
-(IBAction)onExportLuaLog:(id)sender;
- (IBAction)onScrollChange:(id)sender;
- (IBAction)onTopChange:(id)sender;

@end



