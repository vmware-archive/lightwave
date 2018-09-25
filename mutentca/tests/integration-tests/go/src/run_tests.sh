#!/bin/bash

OUTFILE="mutentca_gotest.out"
OUTXML="mutentca_test_report.xml"

usage() {
    echo -e "Multi Tenant CA Testing Script"
    echo -e "Usage:"
    echo -e "./run_tests.sh -s LW_ENDPOINT -n MTCA_ENDPOINT -d LW_DOMAIN -u LW_USERNAME -p LW_PASSWORD"
    echo -e "\t[-o OUTPUT_DIR] [-m GO_TEST_TIMEOUT] [-h]"
    echo -e "Note: Test results output to home directory by default, can be overridden"
}

main() {
    local outdir
    local count=0
    while getopts "s:n:d:u:p:o:m:h" opt ; do
        case "${opt}" in
            s) lwhost="${OPTARG}" ; ((count++)) ;;
            n) mtcahost="${OPTARG}" ; ((count++)) ;;
            d) lwdomain="${OPTARG}" ; ((count++)) ;;
            u) lwuser="${OPTARG}" ; ((count++)) ;;
            p) lwpass="${OPTARG}" ; ((count++)) ;;
            o) outdir="${OPTARG}" ;;
            m) timeout="-test.timeout=${OPTARG}" ;;
            h)
                usage
                exit 0
                ;;
            \?)
                echo "ERROR! Invalid option -${OPTARG}" >&2
                usage
                exit 1
                ;;
            :)
                echo "ERROR! Option -${OPTARG} requires an argument" >&2
                usage
                exit 1
                ;;
        esac
    done

    if [[ "${count}" -lt 5 ]] ; then
        echo "ERROR! Invalid number of arguments - ${count}" >&2
        usage
        exit 1
    fi

    if [[ "${outdir}" == "" ]] ; then
        outdir="${HOME}"
    fi

    (cd mutentcatestsuite; \
        go test -v -args  \
            -lwhost=${lwhost} \
            -mtcahost=${mtcahost} \
            -lwdomain=${lwdomain} \
            -lwuser=${lwuser} \
            -lwpass=${lwpass} \
            ${timeout} 2>&1 \
        | tee ${outdir}/${OUTFILE})

    ${GOPATH}/bin/go2xunit -input ${outdir}/${OUTFILE} -output ${outdir}/${OUTXML}
}

main "$@"
