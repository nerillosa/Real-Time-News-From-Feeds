#!/usr/bin/perl
sub uniq {
    my %seen;
    grep !$seen{$_}++, @_;
}

my $url  = $ARGV[0];

#my $url = "https://www.reuters.com/news/us";

my $html = qx{wget --quiet --output-document=- $url};

my @matches = $html =~ /href=\"(\/article\/[^"]+)/g;

my @filtered = uniq(@matches);

print "@filtered";
