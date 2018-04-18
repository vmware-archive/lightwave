#!/bin/bash
ldapsearch -o ldif-wrap=no -h localhost -p 38900 -x -D $POST_BIND_DN -w $POST_PWD -b "cn=persiststate,cn=raftcontext" -s base vmwRaftFirstLogindex vmwRaftLastApplied -LLL
