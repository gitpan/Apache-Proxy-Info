use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'	=> 'Apache::Proxy::Info',
    'VERSION_FROM' => 'Info.pm', # finds $VERSION
);
