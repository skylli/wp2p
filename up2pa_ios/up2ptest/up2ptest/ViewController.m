//
//  ViewController.m
//  up2ptest
//
//  Created by test on 14-8-7.
//  Copyright (c) 2014年 wilddog. All rights reserved.
//

#import "ViewController.h"
#include "up2pa.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    ulink_test("0000000800000008");
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
