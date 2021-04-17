#!/bin/bash
main()
{
cat << EOF > php.ini
[PHP]
extension=$PWD/pmc.so
EOF
}
main
