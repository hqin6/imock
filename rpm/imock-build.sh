#!/bin/sh

rpmbuild -bb ./imock.spec --define "_topdir `pwd`/.rpmcreate"

find .rpmcreate -name "*.rpm" -exec cp {} ./ \;

rm -rf .rpmcreate;
