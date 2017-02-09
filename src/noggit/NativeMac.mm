//
//  MacDialog.m
//  Noggit
//
//  Created by John Wells on 1/28/17.
//
//

#ifdef __APPLE__

#include <iostream>
#include <string>
#import "Native.hpp"
#import <Cocoa/Cocoa.h>

int Native::showAlertDialog(std::string title, std::string message)
{
    NSAlert *fileAlert = [[NSAlert alloc] init];
    fileAlert.messageText = [NSString stringWithCString:title.c_str() encoding:NSUTF8StringEncoding];
    fileAlert.informativeText = [NSString stringWithCString:message.c_str() encoding:NSUTF8StringEncoding];
    
    int result = [fileAlert runModal];
    [fileAlert release];
    
    return result;
}

//bool NativeMac::checkWoWVersionAtPath(std::string path)
//{
//    NSString *filePath = [NSString stringWithCString:path.c_str() encoding:NSUTF8StringEncoding];
//    NSString *pathWithBundle = [filePath stringByAppendingPathComponent:@"World of Warcraft.app"];
//    NSBundle *wowBundle = [NSBundle bundleWithURL:[NSURL fileURLWithPath:pathWithBundle]];
//    NSDictionary *bundleInfo = [wowBundle infoDictionary];
//    NSString *version = bundleInfo[@"CFBundleVersion"];
//    
//    return [version isEqualToString:@"3.3.5"];
//}
//
std::string Native::getGamePath()
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

std::string Native::getArialPath()
{
	return "/Library/Fonts/Arial.ttf";
}

//
//std::string NativeMac::applicationSupportPath()
//{
//    NSArray *urls = [[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
//    
//    if (urls.count) {
//        return std::string([[[urls[0] relativePath] stringByAppendingString:@"/Noggit/"] UTF8String]);
//    }
//    
//    return "";
//}

#endif
