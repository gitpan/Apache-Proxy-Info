package Apache::Proxy::Info;

use strict;
use vars qw($VERSION @ISA);

use Apache::Constants;

$VERSION = '0.01';
        
sub handler {
	my $r = shift;
	return OK unless ($r->connection->remote_ip eq "127.0.0.1");
	
	#Pass on remote ip
	if (my ($ip) = $r->header_in('X-Forwarded-For') =~ /([^,\s]+)$/) {
	  $r->connection->remote_ip($ip);
	}
	
	#pass on host name
	if (my ($host) = $r->header_in('X-Forwarded-Host') =~ /([^,\s]+)$/) {
	  $r->header_in('Host',$host);
	}
	
	if (my $https = $r->header_in('X-HTTPS')) {
	  $r->subprocess_env('HTTPS',$https);
	}
	
	
return OK;
}

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

42;
__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

Apache::Proxy::Info - Small backend to get information from doc server to mod_perl server

=head1 SYNOPSIS

	PerlModule Apache::Proxy::Info
	PerlTransHandler Aache::Proxy::Info

=head1 DESCRIPTION

This is a simple combination of a small C module for apache and another one in Perl for mod_perl

In lots of cases , you end up with a lightweight docserver in front of a heavyweight mod_perl enabled
server.  This is a good thing for speed and performance (see http://perl.apache.org/guide).

The problem this combination tries to solve is that the mod_perl server doesn't know anything about
the client connection, since it's only talking to a doc server.

Install the C module in the doc server like so

cd apache-src
./configure --with-plenty-of-options -add-module=/path/to/mod_proxy_add_info.c

And you just need to put the Perl module in the mod_perl server
perl Makefile.PL
make && make test && make install

And in httpd.conf
PerlModule Apache::Proxy::Info
PerlTransHandler Apache::Proxy::Info;

Currently 3 pieces of information are passed thru

$r->connection->remote_ip()
$ENV{'HTTPS'}
$r->header_in('Host');


Enjoy!

=head1 AUTHOR

Philippe M. Chiasson <gozer@hbesoftware.com>

=head1 SEE ALSO

perl(1).

=cut
