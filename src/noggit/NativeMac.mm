// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifdef __APPLE__

#include <iostream>
#include <string>
#include <noggit/Log.h>
#import "Native.hpp"
#import <Cocoa/Cocoa.h>

const std::string kNotFoundTitle = "Noggit was unable to locate World of Warcraft.";
const std::string kNotFoundMessage = "Click OK to select the location of your Wrath of the Lich King (3.3.5) installation.";
const std::string kMismatchTitle = "WoW version mismatch";
const std::string kMismatchMessage = "The WoW binary found by Noggit wasn't the expected version (3.3.5). Noggit may crash or behave strangely.";

BOOL checkWoWVersionAtPath(NSString *filePath)
{
    NSString *pathWithBundle = [filePath stringByAppendingPathComponent:@"World of Warcraft.app"];
    NSBundle *wowBundle = [NSBundle bundleWithURL:[NSURL fileURLWithPath:pathWithBundle]];
    NSDictionary *bundleInfo = [wowBundle infoDictionary];
    NSString *version = bundleInfo[@"CFBundleVersion"];

    return [version isEqualToString:@"3.3.5"];
}

std::string Native::showFileChooser()
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.canChooseDirectories = YES;
    openPanel.canChooseFiles = NO;

    if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
        NSString *path = [openPanel.URL relativePath];
        path = [path stringByAppendingString:@"/"];

        return std::string([path UTF8String]);
    }

    return std::string("");
}

std::string Native::getGamePath()
{
    Native::showAlertDialog(kNotFoundTitle, kNotFoundMessage);

    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.canChooseDirectories = YES;
    openPanel.canChooseFiles = NO;

    if ([openPanel runModal] == NSFileHandlingPanelOKButton) {
        NSString *path = [openPanel.URL relativePath];
        path = [path stringByAppendingString:@"/"];

        if (!checkWoWVersionAtPath(path)) {
            Native::showAlertDialog(kMismatchTitle, kMismatchMessage);
            Log << "NoggitCocoa: WoW binary version mismatch" << std::endl;
        }

        return std::string([path UTF8String]);
    }

    return std::string("");
}

std::string Native::getConfigPath()
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *urls = [[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];

    if (urls.count == 0) {
        return "";
    }

    NSString *appSupportPath = [urls[0] relativePath];
    NSString *noggitPath = [appSupportPath stringByAppendingPathComponent:@"Noggit"];
    BOOL isDirectory = NO;
    NSError *error = NULL;

    if (![fileManager fileExistsAtPath:noggitPath isDirectory:&isDirectory]) {
        [fileManager createDirectoryAtPath:noggitPath withIntermediateDirectories:NO attributes:NULL error:&error];

        if (error) {
            LogError << "NoggitCocoa:" << error.localizedFailureReason << std::endl;
            LogError << "NoggitCocoa:" << error.localizedDescription << std::endl;
            [[NSApplication sharedApplication] presentError:error];

            return "";
        }
    } else if (!isDirectory) {
        error = [NSError errorWithDomain:NSCocoaErrorDomain
                                    code:0
                                userInfo:@{ NSLocalizedFailureReasonErrorKey : @"Could not create application support subdirectory.",
                                             NSLocalizedDescriptionKey : @"A file with the name \"Noggit\" already exists. Please delete the file and try again."}];
        LogError << "NoggitCocoa:" << error.localizedFailureReason << std::endl;
        LogError << "NoggitCocoa:" << error.localizedDescription << std::endl;
        [[NSApplication sharedApplication] presentError:error];
    }

    NSString *configPath = [noggitPath stringByAppendingPathComponent:@"noggit.conf"];
    return [configPath UTF8String];
}

#endif
