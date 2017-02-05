//
//  MacDialog.m
//  Noggit
//
//  Created by John Wells on 1/28/17.
//
//

#import "MacDialog.hpp"
#import <Cocoa/Cocoa.h>

extern int showAlertDialog(std::string title, std::string message)
{
    NSAlert *fileAlert = [[NSAlert alloc] init];
    fileAlert.messageText = [NSString stringWithCString:title.c_str() encoding:NSUTF8StringEncoding];
    fileAlert.informativeText = [NSString stringWithCString:message.c_str() encoding:NSUTF8StringEncoding];
    
    [fileAlert runModal];
    
    return 0;
}

bool checkWoWVersionAtPath(std::string path)
{
    NSString *filePath = [[NSString stringWithCString:path.c_str() encoding:NSUTF8StringEncoding] autorelease];
    NSString *pathWithBundle = [filePath stringByAppendingPathComponent:@"World of Warcraft.app"];
    NSBundle *wowBundle = [NSBundle bundleWithURL:[NSURL fileURLWithPath:pathWithBundle]];
    NSDictionary *bundleInfo = [wowBundle infoDictionary];
    NSString *version = bundleInfo[@"CFBundleVersion"];
    
    return [version isEqualToString:@"3.3.5"];
}

std::string requestWoWPath()
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.canChooseDirectories = YES;
    openPanel.canChooseFiles = NO;
    
    if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
        NSString *path = openPanel.URL.relativePath;
        
        return std::string([path UTF8String]);
    }
    
    return std::string("");
}

std::string applicationSupportPath()
{
    NSArray *urls = [[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
    
    if (urls.count) {
        return std::string([[[urls[0] relativePath] stringByAppendingString:@"/Noggit/"] UTF8String]);
    }
    
    return "";
}
