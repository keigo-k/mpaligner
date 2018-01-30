#!/usr/bin/perl

$co=pack("H*", "1e");
$se=pack("H*", "1d");
$de=pack("H*", "1f");
$un=pack("H*", "1c");
$es=pack("H*", "1b");
while(<>){
	$_=~s/\\\\/$es/g;
	$_=~s/\\:/$co/g;
	$_=~s/\\\|/$se/g;
	$_=~s/\\_/$de/g;
	$_=~s/\\ /$un/g;
	print $_;
}

