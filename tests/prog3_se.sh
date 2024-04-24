#!/bin/bash

#!/bin/bash

test -e test_file
ans=$?
echo "File is $(ls)"
rm test_file
exit $ans