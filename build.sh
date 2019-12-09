#!/bin/sh

if [ ! -f ./hmake ]; then
  curl -LO https://github.com/evo-cloud/hmake/releases/download/v1.3.1/hmake-linux-amd64.tar.gz
  tar -xf hmake-linux-amd64.tar.gz
fi

if [ ! -f /usr/local/bin/docker-compose ]; then
  sudo curl -L "https://github.com/docker/compose/releases/download/1.25.0/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
  sudo chmod +x /usr/local/bin/docker-compose
fi

function test() {
  #make .env file
  echo "LIGHTWAVE_DOMAIN=lightwave.local" > support/tests/lightwave/.env
  #make a random pass for test
  echo "LIGHTWAVE_PASS=aA1@`openssl rand -base64 8`" >> support/tests/lightwave/.env
  #remove the quiet flag, skip components that are built and do test
  sudo ./hmake -S pack -S build -S build-lightwave-photon3 test
}

if [[ $1 == 'bootstrap' ]]; then
  ./hmake -q bootstrap-lightwave-photon3
elif [[ $1 == 'build' ]]; then
  ./hmake build-lightwave-photon3
elif [[ $1 == 'pack' ]]; then
  ./hmake -q pack-photon3
elif [[ $1 == 'check' ]]; then
  ./hmake check-lightwave-photon3
else
  test
fi
