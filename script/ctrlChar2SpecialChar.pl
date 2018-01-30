#!/usr/bin/perl


while(<>){
	$_=~s/\x1b/\\\\/g;
	$_=~s/\x1e/\\:/g;
	$_=~s/\x1d/\\|/g;
	$_=~s/\x1f/\\_/g;
	$_=~s/\x1c/\\ /g;
	print $_;
}

