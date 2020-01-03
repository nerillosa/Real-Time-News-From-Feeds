#!/usr/bin/perl
sub uniq {
    my %seen;
    grep !$seen{$_}++, @_;
}

my $section   = $ARGV[0];

my $url = "https://elcomercio.pe/${section}/";

my $html = qx{wget --quiet --output-document=- $url};


my @matches = $html =~ /href=\"(\/$section\/[^"]+)/g;

my @filtered = uniq(@matches);

print "@filtered";
