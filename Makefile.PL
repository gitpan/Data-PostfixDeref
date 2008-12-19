use inc::Module::Install;

use 5.008001;

name        'Data-PostfixDeref';
all_from    'PostfixDeref.pm';

requires    'XSLoader'      => 0;
requires    'Test::More'    => '0.68';

ppport;

if (grep /^$^O$/, qw/MSWin32 aix os2 NetWare/) {
    warn "Shucks, I can't link non-public functions implicitly.\n";
    makemaker_args->{DEFINE} .= ' -DNEED_EXPLICIT_EXPORTS';
}

makemaker_args 
    OBJECT => 'PostfixDeref$(OBJ_EXT) thieved$(OBJ_EXT)',
    INC    => '-I.';

auto_install;
WriteAll;
