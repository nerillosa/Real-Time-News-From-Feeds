#!/usr/bin/perl
sub uniq {
    my %seen;
    grep !$seen{$_}++, @_;
}

my $url = 'https://time.com/';

my $html = qx{wget --quiet --output-document=- $url};

#<a href="/5343771/salmonella-outbreak-raw-turkey/">
#<a href=/5829083/coronavirus-holyoke-veterans-home/ 
my @matches = $html =~ /href=(\/[0-9]+\/[-a-zA-Z0-9]+\/)/g;
my @filtered = uniq(@matches);

print "@filtered";
