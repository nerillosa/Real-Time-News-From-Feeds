#!/usr/bin/perl
sub uniq {
    my %seen;
    grep !$seen{$_}++, @_;
}

my $url = 'https://www.huffingtonpost.com/';

my $html = qx{wget --quiet --output-document=- $url};

my @matches = $html =~ /href=\"(\/entry\/[^"]+)/g;

my @filtered = uniq(@matches);

print "@filtered";
