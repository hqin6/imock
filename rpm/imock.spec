Name: imock
Version:1.0.0
Release: 1
Summary: mock tools for client & server
Group: Private
License: Commercial

AutoReqProv: no

%description
mock tools, include client and server. It can be used in http prototocol, and it support user-define data and proto data format.

%build

cd $OLDPWD;
cd ../src;
make


%install
mkdir -p ${RPM_BUILD_ROOT}/usr/local/bin/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/bin/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/conf/server/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/conf/client/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/conf/tools/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/data/server/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/data/client/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/data/tools/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/proto/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/logs/
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/dev-interface/include
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/dev-interface/libs
mkdir -p ${RPM_BUILD_ROOT}/home/a/imock/dev/libs

cp ../../../src/server/objs/imock-server    ${RPM_BUILD_ROOT}/home/a/imock/bin/
cp ../../../src/client/objs/imock-client    ${RPM_BUILD_ROOT}/home/a/imock/bin/
cp ../../../src/tools/parse/objs/imock-parse      ${RPM_BUILD_ROOT}/home/a/imock/bin/
cp ../../../src/tools/serialize/objs/imock-serialize      ${RPM_BUILD_ROOT}/home/a/imock/bin/
cp ../../../src/tools/diff/objs/imock-diff        ${RPM_BUILD_ROOT}/home/a/imock/bin/
cp ../../../src/tools/pbfile2xml/objs/imock-pbfile2xml   ${RPM_BUILD_ROOT}/home/a/imock/bin/
cp ../../../src/interface/imock-interface.h           ${RPM_BUILD_ROOT}/home/a/imock/dev-interface/include/
cp ../../../src/interface/objs/libimock-interface.a   ${RPM_BUILD_ROOT}/home/a/imock/dev-interface/libs/
cp ../../../src/interface/objs/libimock-interface.so  ${RPM_BUILD_ROOT}/home/a/imock/dev-interface/libs/

cp ../../../conf/imock-server.conf   ${RPM_BUILD_ROOT}/home/a/imock/conf/server/
cp ../../../conf/imock-client.conf   ${RPM_BUILD_ROOT}/home/a/imock/conf/client/
cp ../../../conf/addressbook.proto   ${RPM_BUILD_ROOT}/home/a/imock/proto/
cp ../../../src/tools/diff/imock-diff.conf ${RPM_BUILD_ROOT}/home/a/imock/conf/tools/

cp ../../../data/client/*.xml        ${RPM_BUILD_ROOT}/home/a/imock/data/client/
cp ../../../data/server/*.xml        ${RPM_BUILD_ROOT}/home/a/imock/data/server/
cp ../../../src/tools/diff/*.xml     ${RPM_BUILD_ROOT}/home/a/imock/data/tools/
cp ../../../src/tools/diff/*.txt     ${RPM_BUILD_ROOT}/home/a/imock/data/tools/



%post

rm -f /usr/local/bin/imock-server && ln -s  /home/a/imock/bin/imock-server  /usr/local/bin/imock-server
rm -f /usr/local/bin/imock-client && ln -s  /home/a/imock/bin/imock-client  /usr/local/bin/imock-client
#rm -f /usr/local/bin/ccbuild      && ln -s  /home/a/imock/bin/ccbuild       /usr/local/bin/ccbuild
rm -f /usr/local/bin/imock-parse  && ln -s  /home/a/imock/bin/imock-parse   /usr/local/bin/imock-parse
rm -f /usr/local/bin/imock-serialize  && ln -s  /home/a/imock/bin/imock-serialize   /usr/local/bin/imock-serialize
rm -f /usr/local/bin/imock-diff   && ln -s  /home/a/imock/bin/imock-diff    /usr/local/bin/imock-diff
rm -f /usr/local/bin/imock-pbfile2xml  && ln -s  /home/a/imock/bin/imock-pbfile2xml  /usr/local/bin/imock-pbfile2xml

%postun

%files
%defattr(-,root,root)
/usr/local/bin
/home/a/imock
%config(noreplace) /home/a/imock/conf/server/imock-server.conf
%config(noreplace) /home/a/imock/conf/client/imock-client.conf
%config(noreplace) /home/a/imock/conf/tools/imock-diff.conf
%config(noreplace) /home/a/imock/data/server/*.xml
%config(noreplace) /home/a/imock/data/client/*.xml
%config(noreplace) /home/a/imock/data/tools/*.xml

%changelog



