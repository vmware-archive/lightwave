#!/bin/bash -xe

FAIL=0
SLEEP_TIME=10
HEARTBEAT_INTERVAL=3

main()
{
    declare -ra LWSM_SERVICES=('vmdir'
                               'vmafd'
                               'vmca'
                               'vmdns')

    # adjust vmafd service heartbeat to make script faster
    OLD_HB=$(/opt/likewise/bin/lwregshell list_values '[HKEY_THIS_MACHINE\Services\vmafd\Parameters]' | grep -i HeartbeatInterval | awk '{print $4}' | tr -d '()')
    set_heartbeat_interval ${HEARTBEAT_INTERVAL}

    for SERVICE in ${LWSM_SERVICES[@]}
    do
        test_service ${SERVICE}
    done

    set_heartbeat_interval ${OLD_HB}

    echo ""
    if [ "${FAIL}" != "0" ]; then
        echo "Tests failed"
    else
    echo "Tests passed"
    fi
}

test_service()
{
    SERVICE=$1

    echo "Stopping ${SERVICE} and checking"
    /opt/likewise/bin/lwsm stop ${SERVICE}
    sleep $SLEEP_TIME

    check_endpoint "500"

    # Make sure dependent services started
    echo "Starting ${SERVICE}"
    /opt/likewise/bin/lwsm autostart
    sleep $SLEEP_TIME

    check_endpoint "200"
}

check_endpoint()
{
    EXPECTED=$1

    HTTP_CODE=$(curl -skIX GET -o /dev/null -w "%{http_code}" "https://localhost:443/isAvailable")
    if [ "${HTTP_CODE}" != "${EXPECTED}" ]; then
        echo "Error: expected health endpoint to return [${EXPECTED}], returned [${HTTP_CODE}]"
        FAIL=1
    fi

    return 0
}

set_heartbeat_interval()
{
    INTERVAL=$1
    /opt/likewise/bin/lwregshell set_value '[HKEY_THIS_MACHINE\Services\vmafd\Parameters]' "HeartbeatInterval" ${INTERVAL}
    /opt/likewise/bin/lwsm refresh
    /opt/likewise/bin/lwsm restart vmafd
    sleep $SLEEP_TIME
}

main