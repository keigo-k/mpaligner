#!/usr/bin/perl
#
# mpaligner is program to align string and string.
# Copyright (C) 2010, 2011, 2012 Keigo Kubo
#
# mpaligner is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# mpaligner is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mpaligner.  If not, see <http://www.gnu.org/licenses/>.
#
# Imprement program to separate string for char
# date:   2012/3/13
# author: Keigo Kubo
#
# usage for utf8: cat source/test.utf8.txt | ./script/separate_for_char.pl utf8 source/joint_chars.utf8.txt > test.char_unit
# usage for euc : cat source/test.euc.txt | ./script/separate_for_char.pl euc source/joint_chars.euc.txt > test.char_unit
#
# "joint_chars.{utf8,euc}.txt" includes the list of chars jointed front char.
#

my $moji = '(?:\xA1[\xA1-\xFE]|\xA2[\xA1-\xAE\xBA-\xC1\xCA-\xD0\xDC-\xEA\xF2-\xF9\xFE]|\xA3[\xB0-\xB9\xC1-\xDA\xE1-\xFA]|\xA4[\xA1-\xF3]|\xA5[\xA1-\xF6]|\xA6[\xA1-\xB8\xC1-\xD8]|\xA7[\xA1-\xC1\xD1-\xF1]|\xA8[\xA1-\xC0]|\xAD[\xA1-\xBE\C0-\D6\DF-\FC]|[\xB0-\xCE][\xA1-\xFE]|\xCF[\xA1-\xD3]|[\xD0-\xF3][\xA1-\xFE]|\xF4[\xA1-\xA6]|\x8E[\xA1-\xDF]|[\x02\x03\x09\x0A\x20-\x7E]|\x8F\xA2[\xAF-\xB9]|\x8F\xA2[\xC2-\xC4]|\x8F\xA2[\xEB-\xF1]|\x8F\xA6[\xE1-\xE5]|\x8F\xA6\xE7|\x8F\xA6[\xE9-\xEA]|\x8F\xA6\xEC|\x8F\xA6[\xF1-\xFC]|\x8F\xA7[\xC2-\xCE]|\x8F\xA7[\xF2-\xFE]|\x8F\xA9[\xA1\xA2\xA4\xA6\xA8\xA9\xAB-\xAD\xAF\xB0\xC1-\xD0]|\x8F\xAA[\xA1-\xB8\xBA-\xF7]|\x8F\xAB[\xA1-\xBB\xBD-\xC3\xC5-\xF7]|\x8F[\xB0-\xEC][\xA1-\xFE]|\x8F\xED[\xA1-\xE3])'; # EUC-JP
my $joint_chars=$ARGV[1];
my $char_code=$ARGV[0] || "utf8";
my %joint_list;
if($joint_chars){
	open(FILE, $joint_chars);
	while(<FILE>){
		chomp;
		$joint_list{$_}=1;
	}
}

while(<STDIN>){
	chomp;
	my $str=$_;
	my @list=split(/\t/,$str);
	my $i=0;
	my @result=();
	if(@list==2){
		foreach(@list){
		  my $l;
		  if($char_code eq "utf8"){
		  	$l=&sep_moji_utf8($list[$i]);
		  }else{
		  	$l=&sep_moji_euc($list[$i]);
		  }
		  push(@result,join(" ",@$l));
		  $i++;
		}
	}else{
	  print STDERR "Error:$str\n";
	}

	if($result[0] ne "" && $result[1] ne ""){
	  print $result[0]."\t".$result[1]."\n";
	}else{
	  print STDERR "Error:$str\n";
	}
}

sub sep_moji_utf8{
      my $str = shift;
      my @result = ();
      my $char="";
      my $code_size=0;
      my $code;
      while($str=~/(.)/g){
    	$code=$1;
		$char.=$code;

		$code=unpack("B8", $code);
		if($code_size==0){
			if($code=~/^0/){
				$code_size=1
			}elsif($code=~/^110/){
				$code_size=2
			}elsif($code=~/^1110/){
				$code_size=3
			}elsif($code=~/^11110/){
				$code_size=4
			}elsif($code=~/^111110/){
				$code_size=5
			}else{
				$code_size=6
			}
 		}

    	$code_size--;

		if($code_size==0){
			if($char=~/^(\x5f|\x7c|\x3a|\x20|\x5c)$/){
				push(@result,"\\".$char);
			}elsif(defined $joint_list{$char}){
				my $tmp = pop @result;
				push(@result, $tmp.$char);
			}else{
				push(@result, $char);
			}
			$char="";
		}
	}

	return(\@result);
}

sub sep_moji_euc{
      my $str = shift;
      my @result = ();
      while($str=~/($moji)/g){
		my $char = $1;
		if($char=~/^(\x5f|\x7c|\x3a|\x20|\x5c)$/){
			push(@result,"\\".$char);
		}elsif(defined $joint_list{$char}){
			my $tmp = pop @result;
			push(@result, $tmp.$char);
		}else{
			push(@result, $char);
		}
      }

      return(\@result);
}


