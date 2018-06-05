#!/bin/sh

###
# Verifies VMIdentity is healthy and ready to serve on this host
is_idm_healthy()
{
  echo "Starting IDM health check"

  declare -ra IDM_URIS=('/sts/STSService'
                        '/afd/vecs/ssl'
                        '/idm/'
                        '/openidconnect/jwks')

  # retry 3 times every 5 seconds until all IDM endpoints respond 200
  RETRY=0
  while [[ ${RETRY} -lt 3 ]]
  do
    sleep 5
    echo "Attempt ${RETRY}"

    HEALTHY=0
    for URI in ${IDM_URIS[@]}
    do
      echo "Checking endpoint https://localhost:443${URI}"
      HTTP_CODE=$(curl -skIX GET -o /dev/null -w "%{http_code}" "https://localhost:443${URI}")
      if [[ ${HTTP_CODE} -eq 200 ]]
      then
        echo "Healthy"
        ((HEALTHY+=1))
      else
        echo "Not healthy (${HTTP_CODE})"
      fi
    done

    if [[ ${#IDM_URIS[@]} -eq ${HEALTHY} ]]
    then
      echo "IDM health check passed"
      return 0
    fi
    ((RETRY+=1))
  done

  echo "IDM health check failed"
  return 1
}
