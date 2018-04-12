#!/bin/bash

pub_tcp_ports=( "443" "636" "9092" )

priv_tcp_ports=( "53" "88" "389" "636" "2012" "2014" "2015" "2020" "7478" "7778" )
priv_udp_ports=( "53" "88" )
priv_icmp_port="-1"

elb_tcp_ports=( "88" "443" "636" "2012" "2014" "2015" "2020" "7778" "9092" )

tcp_listeners=( "88" "636" "2012" "2014" "2015" "2020" "7778" "9092" )
https_listeners=( "443" )

usage() {
    echo -e "Usage:"
    echo -e "1) ./aws_security_utility.sh add-security-group-rules -s SECURITY_GROUP_ID -t SG_TYPE(priv/pub/elb) -i CIDR_IP_RANGE"
    echo -e "\t[-p PROFILE(default: photon-infra)] [-r REGION(default: us-west-2)]"
    echo -e "2) ./aws_security_utility.sh add-elb-listeners -l ELB_NAME -c SSL_CERTIFICATE_ARN"
    echo -e "\t[-p PROFILE(default: photon-infra)] [-r REGION(default: us-west-2)]"
}

update_security_groups_pub() {
    sg_id=$1
    cidr=$2
    profile=$3
    region=$4

    for i in "${pub_tcp_ports[@]}"
    do
        aws ec2 authorize-security-group-ingress --profile $profile --region $region --group-id $sg_id --protocol tcp --port $i --cidr $cidr
        if [[ $? -eq 0 ]] ; then
            echo -e "TCP Port ${i} Successfully Added"
        fi
    done
}

update_security_groups_priv() {
    sg_id=$1
    cidr=$2
    profile=$3
    region=$4

    for i in "${priv_tcp_ports[@]}"
    do
        aws ec2 authorize-security-group-ingress --profile $profile --region $region --group-id $sg_id --protocol tcp --port $i --cidr $cidr
        if [[ $? -eq 0 ]] ; then
            echo -e "TCP Port ${i} Successfully Added"
        fi
    done

    for i in "${priv_udp_ports[@]}"
    do
        aws ec2 authorize-security-group-ingress --profile $profile --region $region --group-id $sg_id --protocol udp --port $i --cidr $cidr
        if [[ $? -eq 0 ]] ; then
            echo -e "UDP Port ${i} Successfully Added"
        fi
    done

    aws ec2 authorize-security-group-ingress --profile $profile --region $region --group-id $sg_id --protocol icmp --port $priv_icmp_port --cidr $cidr
    if [[ $? -eq 0 ]] ; then
        echo -e "ICMP Rule Successfully Added"
    fi
}

update_security_groups_elb() {
    sg_id=$1
    cidr=$2
    profile=$3
    region=$4

    for i in "${elb_tcp_ports[@]}"
    do
        aws ec2 authorize-security-group-ingress --profile $profile --region $region --group-id $sg_id --protocol tcp --port $i --cidr $cidr
        if [[ $? -eq 0 ]] ; then
            echo -e "TCP Port ${i} Successfully Added"
        fi
    done
}

update_elb_listeners() {
    elb=$1
    cert=$2
    profile=$3
    region=$4

    for i in "${tcp_listeners[@]}"
    do
        aws elb create-load-balancer-listeners --profile $profile --region $region --load-balancer-name $elb --listeners "Protocol=TCP,LoadBalancerPort=${i},InstanceProtocol=TCP,InstancePort=${i}"
        if [[ $? -eq 0 ]] ; then
            echo -e "TCP Port ${i} Successfully Added"
        fi
    done

    for i in "${https_listeners[@]}"
    do
        aws elb create-load-balancer-listeners --profile $profile --region $region --load-balancer-name $elb --listeners "Protocol=HTTPS,LoadBalancerPort=${i},InstanceProtocol=HTTPS,InstancePort=${i},SSLCertificateId=${cert}"
        if [[ $? -eq 0 ]] ; then
            echo -e "HTTPS Port ${i} Successfully Added"
        fi
    done
}

add_security_group_rules() {
    local count=0
    while getopts "s:t:i:p:r:" opt ; do
        case "${opt}" in
            s) sg_id="${OPTARG}" ; ((count++)) ;;
            t) sg_type="${OPTARG}" ; ((count++)) ;;
            i) cidr="${OPTARG}" ; ((count++)) ;;
            p) profile="${OPTARG}" ;;
            r) region="${OPTARG}" ;;
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

    if [[ "${count}" -lt 3 ]] ; then
        echo "ERROR! Invalid number of arguments - ${count}" >&2
        usage
        exit 1
    fi

    if ! [[ "${cidr}" =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}/([0-9]|1[0-9]|2[0-4])$ ]] ; then
        echo "ERROR! Invalid argument. CIDR Format: x.x.x.x/x (0.0.0.0/0 to 255.255.255.255/24)" >&2
        exit 1
    fi

    if [[ "${profile}" == "" ]] ; then
        profile="photon-infra"
    fi

    if [[ "${region}" == "" ]] ; then
        region="us-west-2"
    fi

    if [[ "${sg_type}" == "priv" ]] ; then
        update_security_groups_priv $sg_id $cidr $profile $region
    elif [[ "${sg_type}" == "pub" ]] ; then
        update_security_groups_pub $sg_id $cidr $profile $region
    elif [[ "${sg_type}" == "elb" ]] ; then
        update_security_groups_elb $sg_id $cidr $profile $region
    else
        echo "ERROR! Invalid argument. SG_Type can be priv/pub/elb" >&2
        exit 1
    fi
}

add_elb_listeners() {
    local count=0
    while getopts "l:c:p:r:" opt ; do
        case "${opt}" in
            l) elb="${OPTARG}" ; ((count++)) ;;
            c) cert_id="${OPTARG}" ; ((count++)) ;;
            p) profile="${OPTARG}" ;;
            r) region="${OPTARG}" ;;
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

    if [[ "${count}" -lt 2 ]] ; then
        echo "ERROR! Invalid number of arguments - ${count}" >&2
        usage
        exit 1
    fi

    if [[ "${profile}" == "" ]] ; then
        profile="photon-infra"
    fi

    if [[ "${region}" == "" ]] ; then
        region="us-west-2"
    fi

    update_elb_listeners $elb $cert_id $profile $region
}

main() {
    subcommand=$1; shift

    if [[ "${subcommand}" == "add-security-group-rules" ]] ; then
        add_security_group_rules "$@"
    elif [[ "${subcommand}" == "add-elb-listeners" ]] ; then
        add_elb_listeners "$@"
    else
        echo "ERROR! Invalid subcommand -${subcommand}" >&2
        usage
        exit 1
    fi
}

main "$@"
