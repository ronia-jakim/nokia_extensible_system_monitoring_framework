g++ ./src/server/server.cpp -DTEST -o ./src/server/server
gcc ./src/bash_client/client.c -DTEST -o ./src/bash_client/client

chmod +x ./src/server/test.sh

echo "testing..."
echo " "
echo " "

test_outcome=$(./src/server/test.sh)

echo " "

echo "test $test_outcome"


#cmake .
#make all
