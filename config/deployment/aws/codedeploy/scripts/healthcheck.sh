#!/bin/sh

###
# Verifies VMIdentity is healthy and ready to serve on this host
is_idm_healthy()
{
  declare -ra IDM_URIS=('/sts/STSService'
                        '/afd/vecs/ssl'
                        '/idm/'
                        '/openidconnect/jwks')

  # retry 3 times every 5 seconds until all IDM endpoints respond 200
  RETRY=0
  while [[ ${RETRY} -lt 3 ]]
  do
    sleep 5

    HEALTHY=0
    for URI in "${IDM_URIS[@]}"
    do
      if [[ 200 -eq $(curl -skIX GET -o /dev/null -w "%{http_code}" "https://localhost:443${URI}") ]]
      then
        (( HEALTH++ ))
      fi
    done

    if [[ ${#IDM_URIS[@]} -eq ${HEALTH} ]]
    then
        return 0
    fi
    (( RETRY++ ))
  done

  return 1
}
