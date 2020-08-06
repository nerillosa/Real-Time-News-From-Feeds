#!/usr/bin/perl
sub uniq {
    my %seen;
    grep !$seen{$_}++, @_;
}

my $section   = $ARGV[0];

my $url = "https://larepublica.pe/${section}/";

my $html = qx{wget --quiet --output-document=- $url};

my @matches = $html =~ /\"canonical_url\":\"(\/$section\/[^"]+)/g;

my @filtered = uniq(@matches);

print "@filtered";
