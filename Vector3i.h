//
//  Vector3i.h
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface Vector3i : NSObject {
    @private
    int coords[3];
}

+ (Vector3i *)nullVector;

- (id)initWithIntVector:(Vector3i *)vector;
- (id)initWithFloatVector:(Vector3f *)vector;
- (id)initWithIntX:(int)xCoord y:(int)yCoord z:(int) zCoord;

- (int)x;
- (int)y;
- (int)z;

- (void)setX:(int)xCoord;
- (void)setY:(int)yCoord;
- (void)setZ:(int)zCoord;

- (void)setInt:(Vector3i *)vector;
- (void)setFloat:(Vector3f *)vector;

- (BOOL)null;

- (void)add:(Vector3i *)addend;
- (void)addX:(int)xAddend Y:(int)yAddend Z:(int)zAddend;
- (void)sub:(Vector3i *)subtrahend;
- (void)subX:(int)xSubtrahend Y:(int)ySubtrahend Z:(int)zSubtrahend;

- (int)dot:(Vector3i *)m;
- (void)cross:(Vector3i *)m;
- (void)scale:(float)f;

- (NSComparisonResult)compareToVector:(Vector3i *)vector;
- (BOOL)isEqualToVector:(Vector3i *)vector;
@end
