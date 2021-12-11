
#import "ConsoleWindowController.h"

@interface ConsoleWindowController ()

@end

#define SKIP_LINES_COUNT    3
#define MAX_LINE_LEN        4096
#define MAX_LINES_COUNT     200

@implementation ConsoleWindowController
@synthesize textView;

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self){
        // Initialization code here.
        linesCount = [[NSMutableArray arrayWithCapacity:MAX_LINES_COUNT + 1] retain];
        _reserveMutableArray = [[NSMutableArray arrayWithCapacity:MAX_LINES_COUNT + 1] retain];
        _reserveStringArray = [[NSMutableArray arrayWithCapacity:MAX_LINES_COUNT + 1] retain];
        
        NSFont *font = [NSFont fontWithName:@"Monaco" size:12.0];
        _fontAttrib = [[NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName]retain];
    }
    //NSColor *color = window.backgroundColor;
    //window.backgroundColor = NSColor.whiteColor;
    //window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantLight];
    //window.
    _reserveCount = 0;
    _isReceiveMsgNormal = YES;
    return self;
}

- (void)dealloc
{
    [linesCount release];
    [_reserveMutableArray release];
    [_reserveStringArray release];
    [_fontAttrib release];
    [super dealloc];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    if(!self.window.appearance || self.window.appearance.name != NSAppearanceNameVibrantLight)
        self.window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantLight];
}

- (void) trace:(NSString*)msg
{
    //发现很多时候有大量的空日志传入  ,此时直接过滤掉更好
    if(!msg || !msg.length) return;
    //首先需要判断是否已经停止了s日志输出
    if(!_isReceiveMsgNormal){
        if(_reserveCount >= SKIP_LINES_COUNT && [msg length] > MAX_LINE_LEN)
            msg = [NSString stringWithFormat:@"%@ ...", [msg substringToIndex:MAX_LINE_LEN - 4]];
        _reserveCount++;
        //NSFont *font = [NSFont fontWithName:@"Monaco" size:12.0];
        //NSDictionary *attrsDictionary = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
        NSAttributedString *string = [[NSAttributedString alloc] initWithString:msg attributes:_fontAttrib];//attrsDictionary];
        NSNumber *len = [NSNumber numberWithUnsignedInteger:[string length]];
        [_reserveMutableArray addObject:len];
        [_reserveStringArray addObject:string];
        
        if ([_reserveStringArray count] >= MAX_LINES_COUNT){
            [_reserveMutableArray removeObjectAtIndex:0];
            [_reserveStringArray removeObjectAtIndex:0];
            --_reserveCount;
        }
        return;
    }
    //需要同时计算是否需要将原来保存的消息与新的消息合并起来
    if(_reserveMutableArray.count != 0 ){
        //当前还不能确定是否需要一下子就降所有的消息发出去,因为这可能会引起卡顿,这里先采用不同的策略测试一下结果如何
        NSTextStorage *storage = [textView textStorage];
        [storage beginEditing];
        while(_reserveMutableArray.count > 0){
            NSAttributedString *string = [_reserveStringArray objectAtIndex:0];
            [storage appendAttributedString:string];
            [_reserveStringArray removeObjectAtIndex:0];
            
            NSNumber *row_num = [_reserveMutableArray objectAtIndex:0];
            [linesCount addObject:row_num];
            [_reserveMutableArray removeObjectAtIndex:0];
            ++traceCount;
            
            if(linesCount.count >= MAX_LINES_COUNT){
                NSNumber *first_row = [linesCount objectAtIndex:0];
                [storage deleteCharactersInRange:NSMakeRange(0, [first_row unsignedIntegerValue])];
                [linesCount removeObjectAtIndex:0];
                
                --traceCount;
            }
            
            --_reserveCount;
        }
        [storage endEditing];
    }
    
    if (traceCount >= SKIP_LINES_COUNT && [msg length] > MAX_LINE_LEN)
    {
        msg = [NSString stringWithFormat:@"%@ ...", [msg substringToIndex:MAX_LINE_LEN - 4]];
    }
    traceCount++;
    //NSFont *font = [NSFont fontWithName:@"Monaco" size:12.0];
    //NSDictionary *attrsDictionary = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
    NSAttributedString *string = [[NSAttributedString alloc] initWithString:msg attributes:_fontAttrib];//attrsDictionary];
    NSNumber *len = [NSNumber numberWithUnsignedInteger:[string length]];
    [linesCount addObject:len];

	NSTextStorage *storage = [textView textStorage];
	[storage beginEditing];
	[storage appendAttributedString:string];

    if ([linesCount count] >= MAX_LINES_COUNT)
    {
        len = [linesCount objectAtIndex:0];
        [storage deleteCharactersInRange:NSMakeRange(0, [len unsignedIntegerValue])];
        [linesCount removeObjectAtIndex:0];
        
        --traceCount;
    }

	[storage endEditing];
    
    NSRange select_range = self.textView.selectedRange;
    NSInteger text_length =  self.textView.string.length;
    if(select_range.location == text_length && !select_range.length)
        [self changeScroll];
    
    //NSArray<NSValue *> *text_array = self.textView.selectedRanges;
    //if(!text_array || !text_array.count)
    //    [self changeScroll];
}

- (void) changeScroll
{
    BOOL scroll = [checkScroll state] == NSOnState;
    if(scroll)
    {
        [self.textView scrollRangeToVisible: NSMakeRange(self.textView.string.length, 0)];
    }
}

- (IBAction)onClear:(id)sender
{
    NSTextStorage *storage = [textView textStorage];
    [storage setAttributedString:[[[NSAttributedString alloc] initWithString:@"" attributes:_fontAttrib] autorelease]];
    
    traceCount = 0;
    [linesCount removeAllObjects];
}

- (IBAction)onPause:(id)sender {
    _isReceiveMsgNormal = !_isReceiveMsgNormal;
    NSButton *button = sender;
    [button setTitle:_isReceiveMsgNormal?@"pause log":@"resume log"];
}

- (IBAction)onScrollChange:(id)sender
{
    [self changeScroll];
}

- (IBAction)onTopChange:(id)sender
{
    BOOL isTop = [topCheckBox state] == NSOnState;
    if(isTop)
    {
        [self.window setLevel:NSFloatingWindowLevel];
    }
    else
    {
        [self.window setLevel:NSNormalWindowLevel];
    }
}

-(IBAction)onExportLuaLog:(id)sender
{
    //export_lua_log_operate();
}

@end
