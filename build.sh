g++ ./src/server/server.cpp -DTEST -o ./src/server/server
gcc ./src/bash_client/client.c -DTEST -o ./src/bash_client/client

echo "testing..."
echo " "
echo " "

test_outcome=$(./src/server/server | ./src/bash_client/client)

echo " "

echo "test $test_outcome"


#cmake .
#make all
