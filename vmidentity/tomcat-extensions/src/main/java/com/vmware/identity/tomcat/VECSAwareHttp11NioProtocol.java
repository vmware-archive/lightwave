package com.vmware.identity.tomcat;

import org.apache.coyote.http11.Http11NioProtocol;

public class VECSAwareHttp11NioProtocol extends Http11NioProtocol {

    @Override
    protected String getSslImplemenationShortName() {
        return "Vecs Aware JSSE";
    }

}
