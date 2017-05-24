webpackJsonp([1,4],{

/***/ 141:
/***/ (function(module, exports) {

function webpackEmptyContext(req) {
	throw new Error("Cannot find module '" + req + "'.");
}
webpackEmptyContext.keys = function() { return []; };
webpackEmptyContext.resolve = webpackEmptyContext;
module.exports = webpackEmptyContext;
webpackEmptyContext.id = 141;


/***/ }),

/***/ 142:
/***/ (function(module, exports, __webpack_require__) {

"use strict";

Object.defineProperty(exports, "__esModule", { value: true });
__webpack_require__(162);
var platform_browser_dynamic_1 = __webpack_require__(157);
var core_1 = __webpack_require__(10);
var environment_1 = __webpack_require__(161);
var _1 = __webpack_require__(160);
if (environment_1.environment.production) {
    core_1.enableProdMode();
}
platform_browser_dynamic_1.platformBrowserDynamic().bootstrapModule(_1.AppModule);
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/main.js.map

/***/ }),

/***/ 158:
/***/ (function(module, exports, __webpack_require__) {

"use strict";

var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
Object.defineProperty(exports, "__esModule", { value: true });
var platform_browser_1 = __webpack_require__(33);
var core_1 = __webpack_require__(10);
var forms_1 = __webpack_require__(88);
var http_1 = __webpack_require__(156);
var clarity_angular_1 = __webpack_require__(90);
var app_component_1 = __webpack_require__(89);
var utils_module_1 = __webpack_require__(165);
var app_routing_1 = __webpack_require__(159);
var AppModule = (function () {
    function AppModule() {
    }
    return AppModule;
}());
AppModule = __decorate([
    core_1.NgModule({
        declarations: [
            app_component_1.AppComponent
        ],
        imports: [
            platform_browser_1.BrowserModule,
            forms_1.FormsModule,
            http_1.HttpModule,
            clarity_angular_1.ClarityModule.forRoot(),
            utils_module_1.UtilsModule,
            app_routing_1.ROUTING
        ],
        providers: [],
        bootstrap: [app_component_1.AppComponent]
    })
], AppModule);
exports.AppModule = AppModule;
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/app/app.module.js.map

/***/ }),

/***/ 159:
/***/ (function(module, exports, __webpack_require__) {

"use strict";

Object.defineProperty(exports, "__esModule", { value: true });
var router_1 = __webpack_require__(47);
exports.ROUTES = [
    { path: '', redirectTo: 'home', pathMatch: 'full' }
];
exports.ROUTING = router_1.RouterModule.forRoot(exports.ROUTES);
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/app/app.routing.js.map

/***/ }),

/***/ 160:
/***/ (function(module, exports, __webpack_require__) {

"use strict";

function __export(m) {
    for (var p in m) if (!exports.hasOwnProperty(p)) exports[p] = m[p];
}
Object.defineProperty(exports, "__esModule", { value: true });
__export(__webpack_require__(89));
__export(__webpack_require__(158));
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/app/index.js.map

/***/ }),

/***/ 161:
/***/ (function(module, exports, __webpack_require__) {

"use strict";
// The file contents for the current environment will overwrite these during build.
// The build system defaults to the dev environment which uses `environment.ts`, but if you do
// `ng build --env=prod` then `environment.prod.ts` will be used instead.
// The list of which env maps to which file can be found in `angular-cli.json`.

Object.defineProperty(exports, "__esModule", { value: true });
exports.environment = {
    production: true
};
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/environments/environment.js.map

/***/ }),

/***/ 162:
/***/ (function(module, exports, __webpack_require__) {

"use strict";

Object.defineProperty(exports, "__esModule", { value: true });
// This file includes polyfills needed by Angular 2 and is loaded before
// the app. You can add your own extra polyfills to this file.
__webpack_require__(179);
__webpack_require__(172);
__webpack_require__(168);
__webpack_require__(174);
__webpack_require__(173);
__webpack_require__(171);
__webpack_require__(170);
__webpack_require__(178);
__webpack_require__(167);
__webpack_require__(166);
__webpack_require__(176);
__webpack_require__(169);
__webpack_require__(177);
__webpack_require__(175);
__webpack_require__(180);
__webpack_require__(358);
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/polyfills.js.map

/***/ }),

/***/ 163:
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/*
 * Hack while waiting for https://github.com/angular/angular/issues/6595 to be fixed.
 */

var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var __metadata = (this && this.__metadata) || function (k, v) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(k, v);
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = __webpack_require__(10);
var router_1 = __webpack_require__(47);
var HashListener = (function () {
    function HashListener(route) {
        var _this = this;
        this.route = route;
        this.sub = this.route.fragment.subscribe(function (f) {
            _this.scrollToAnchor(f, false);
        });
    }
    HashListener.prototype.ngOnInit = function () {
        this.scrollToAnchor(this.route.snapshot.fragment, false);
    };
    HashListener.prototype.scrollToAnchor = function (hash, smooth) {
        if (smooth === void 0) { smooth = true; }
        if (hash) {
            var element = document.querySelector("#" + hash);
            if (element) {
                element.scrollIntoView({
                    behavior: smooth ? "smooth" : "instant",
                    block: "start"
                });
            }
        }
    };
    HashListener.prototype.ngOnDestroy = function () {
        this.sub.unsubscribe();
    };
    return HashListener;
}());
HashListener = __decorate([
    core_1.Directive({
        selector: "[hash-listener]",
        host: {
            "[style.position]": "'relative'"
        }
    }),
    __metadata("design:paramtypes", [typeof (_a = typeof router_1.ActivatedRoute !== "undefined" && router_1.ActivatedRoute) === "function" && _a || Object])
], HashListener);
exports.HashListener = HashListener;
var _a;
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/utils/hash-listener.directive.js.map

/***/ }),

/***/ 164:
/***/ (function(module, exports, __webpack_require__) {

"use strict";
/*
 * Hack while waiting for https://github.com/angular/angular/issues/6595 to be fixed.
 */

var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var __metadata = (this && this.__metadata) || function (k, v) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(k, v);
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = __webpack_require__(10);
var router_1 = __webpack_require__(47);
var ScrollSpy = (function () {
    function ScrollSpy(renderer) {
        this.renderer = renderer;
        this.anchors = [];
        this.throttle = false;
    }
    Object.defineProperty(ScrollSpy.prototype, "links", {
        set: function (routerLinks) {
            var _this = this;
            this.anchors = routerLinks.map(function (routerLink) { return "#" + routerLink.fragment; });
            this.sub = routerLinks.changes.subscribe(function () {
                _this.anchors = routerLinks.map(function (routerLink) { return "#" + routerLink.fragment; });
            });
        },
        enumerable: true,
        configurable: true
    });
    ScrollSpy.prototype.handleEvent = function () {
        var _this = this;
        this.scrollPosition = this.scrollable.scrollTop;
        if (!this.throttle) {
            window.requestAnimationFrame(function () {
                var currentLinkIndex = _this.findCurrentAnchor() || 0;
                _this.linkElements.forEach(function (link, index) {
                    _this.renderer.setElementClass(link.nativeElement, "active", index === currentLinkIndex);
                });
                _this.throttle = false;
            });
        }
        this.throttle = true;
    };
    ScrollSpy.prototype.findCurrentAnchor = function () {
        for (var i = this.anchors.length - 1; i >= 0; i--) {
            var anchor = this.anchors[i];
            if (this.scrollable.querySelector(anchor) && this.scrollable.querySelector(anchor).offsetTop <= this.scrollPosition) {
                return i;
            }
        }
    };
    ScrollSpy.prototype.ngOnInit = function () {
        this.scrollable.addEventListener("scroll", this);
    };
    ScrollSpy.prototype.ngOnDestroy = function () {
        this.scrollable.removeEventListener("scroll", this);
        if (this.sub) {
            this.sub.unsubscribe();
        }
    };
    return ScrollSpy;
}());
__decorate([
    core_1.Input("scrollspy"),
    __metadata("design:type", Object)
], ScrollSpy.prototype, "scrollable", void 0);
__decorate([
    core_1.ContentChildren(router_1.RouterLinkWithHref, { descendants: true }),
    __metadata("design:type", typeof (_a = typeof core_1.QueryList !== "undefined" && core_1.QueryList) === "function" && _a || Object),
    __metadata("design:paramtypes", [typeof (_b = typeof core_1.QueryList !== "undefined" && core_1.QueryList) === "function" && _b || Object])
], ScrollSpy.prototype, "links", null);
__decorate([
    core_1.ContentChildren(router_1.RouterLinkWithHref, { descendants: true, read: core_1.ElementRef }),
    __metadata("design:type", typeof (_c = typeof core_1.QueryList !== "undefined" && core_1.QueryList) === "function" && _c || Object)
], ScrollSpy.prototype, "linkElements", void 0);
ScrollSpy = __decorate([
    core_1.Directive({
        selector: "[scrollspy]",
    }),
    __metadata("design:paramtypes", [typeof (_d = typeof core_1.Renderer !== "undefined" && core_1.Renderer) === "function" && _d || Object])
], ScrollSpy);
exports.ScrollSpy = ScrollSpy;
var _a, _b, _c, _d;
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/utils/scrollspy.directive.js.map

/***/ }),

/***/ 165:
/***/ (function(module, exports, __webpack_require__) {

"use strict";

var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = __webpack_require__(10);
var hash_listener_directive_1 = __webpack_require__(163);
var scrollspy_directive_1 = __webpack_require__(164);
var clarity_angular_1 = __webpack_require__(90);
var common_1 = __webpack_require__(40);
var UtilsModule = (function () {
    function UtilsModule() {
    }
    return UtilsModule;
}());
UtilsModule = __decorate([
    core_1.NgModule({
        imports: [
            common_1.CommonModule,
            clarity_angular_1.ClarityModule.forChild()
        ],
        declarations: [
            hash_listener_directive_1.HashListener,
            scrollspy_directive_1.ScrollSpy
        ],
        exports: [
            hash_listener_directive_1.HashListener,
            scrollspy_directive_1.ScrollSpy
        ]
    })
], UtilsModule);
exports.UtilsModule = UtilsModule;
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/utils/utils.module.js.map

/***/ }),

/***/ 320:
/***/ (function(module, exports, __webpack_require__) {

exports = module.exports = __webpack_require__(38)(false);
// imports


// module
exports.push([module.i, ".clr-icon.clr-clarity-logo {\n  background-image: url(/lightwave/images/vmw_oss.svg); }\n\n.hero {\n  background-color: #ddd;\n  left: -24px;\n  padding-bottom: 2em;\n  padding-top: 2em;\n  overflow-x: hidden;\n  position: relative;\n  text-align: center;\n  top: -24px; }\n  .hero .btn-custom {\n    display: inline-block;\n    text-align: center;\n    margin: auto; }\n\n@media (min-width: 320px) {\n  .content-area {\n    overflow-x: hidden; }\n  .hero {\n    width: 100vw; } }\n\n@media (min-width: 768px) {\n  .content-area {\n    overflow-x: hidden; }\n  .hero {\n    width: 110%; } }\n\n.hero-image img {\n  max-width: 360px; }\n\n.icon {\n  display: inline-block;\n  height: 32px;\n  vertical-align: middle;\n  width: 32px; }\n  .icon.icon-github {\n    background: url(/lightwave/images/github_icon.svg) no-repeat left -2px; }\n\n.nav-group label {\n  display: block;\n  margin-bottom: 1em; }\n\n.sidenav .nav-link {\n  padding: 3px 6px; }\n  .sidenav .nav-link:hover {\n    background: #eee; }\n  .sidenav .nav-link.active {\n    background: #d9e4ea;\n    color: #000; }\n\n.section {\n  padding: .5em 0; }\n\n.contributor {\n  border-radius: 50%;\n  border: 1px solid #ccc;\n  margin-bottom: 1.5em;\n  margin-right: 1em;\n  max-width: 64px;\n  text-decoration: none; }\n\n@media (min-width: 320px) {\n  #license {\n    padding-bottom: 20vh; } }\n\n@media (min-width: 768px) {\n  #license {\n    padding-bottom: 77vh; } }\n\n.row:after {\n  clear: both;\n  content: \"\";\n  display: table; }\n", ""]);

// exports


/*** EXPORTS FROM exports-loader ***/
module.exports = module.exports.toString();

/***/ }),

/***/ 329:
/***/ (function(module, exports) {

module.exports = "<clr-main-container>\n    <header class=\"header header-6\">\n        <div class=\"branding\">\n            <a href=\"https://vmware.github.io/\" class=\"nav-link\">\n                <span class=\"clr-icon clr-clarity-logo\"></span>\n                <span class=\"title\">VMware&reg; Open Source Program Office</span>\n            </a>\n        </div>\n    </header>\n    <div class=\"content-container\">\n        <div id=\"content-area\" class=\"content-area\" hash-listener #scrollable>\n            <div class=\"hero\">\n                <div class=\"hero-image\"><img src=\"images/lightwave.png\" alt=\"VMware Lightwave&trade;\"></div>\n                <h3>Security for Container Ecosystem</h3>\n                <p><a href=\"https://github.com/vmware/lightwave\" class=\"btn btn-primary\"><i class=\"icon icon-github\"></i> Fork Lightwave&trade;</a></p>\n            </div>\n            <div id=\"overview\" class=\"section\">\n                <h2>What is Lightwave</h2>\n\n                <p>Project Lightwave is an open source project comprised of enterprise-grade, identity and access management services targeting critical security, governance, and compliance challenges for Cloud-Native Apps within the enterprise. Through integration with Project Photon, Project Lightwave can provide security and governance for container workloads. Project Lightwave can also serve a variety of use cases such as single sign-on, authentication, authorization and certificate authority, as well as certificate key management services across the entire infrastructure and application stack. Project Lightwave is based on a production quality code and an enterprise-grade architecture that is multi-tenant, scalable, and highly available multi master replication topology.</p>\n\n                <p>Project Lightwave is made up of the following key identity infrastructure elements:</p>\n\n                <br>\n\n                <ul>\n                    <li><strong>Lightwave Directory Service</strong> - standards based, multi-tenant, multi-master, highly scalable LDAP v3 directory service enables an enterprise’s infrastructure to be used by the most-demanding applications as well as by multiple teams.</li>\n                    <li><strong>Lightwave Certificate Authority</strong> - directory integrated certificate authority helps to simplify certificate-based operations and key management across the infrastructure.</li>\n                    <li><strong>Lightwave Certificate Store</strong> - endpoint certificate store to store certificate credentials.</li>\n                    <li><strong>Lightwave Authentication Services</strong> - cloud authentication services with support for Kerberos, OAuth 2.0/OpenID Connect, SAML and WSTrust enable interoperability with other standards-based technologies in the data center.</li>\n                </ul>\n            </div>\n\n            <div id=\"gettingLightwave\" class=\"section\">\n                <h2>Getting Lightwave&trade;</h2>\n\n                <p>You can download the latest Project Lightwave source code from <a href=\"https://github.com/vmware/lightwave\">Github</a>. Project Lightwave has been validated with Project Photon and should work with most Linux container run time hosts. You can also get Lightwave RPMs from <a href=\"https://bintray.com/vmware/lightwave\">Bintray</a>.</p>\n            </div>\n\n            <div id=\"gettingStarted\" class=\"section\">\n                <h2>Getting Started</h2>\n                \n                <p>We’ve provided a few guides to help get you started:</p>\n\n                <p>You can download the latest Project Lightwave source code and documentation including the <a href=\"https://github.com/vmware/lightwave/blob/master/README.md\">README</a>. Project Lightwave has been validated with Project Photon and should work with most Linux container run time hosts.</p>\n            </div>\n\n            <div id=\"support\" class=\"section\">\n                <h2>Support</h2>\n                \n                <p>Lightwave&trade; is released as open source software and comes with no commercial support.</p>\n\n                <p>But since we want to ensure success and recognize that Lightwave&trade; consumers might fall into a range of roles - from developers that are steeped in the conventions of open-source to customers that are more accustomed to VMware commercial offerings, we offer several methods of engaging with the Lightwave&trade; team and community.</p>\n\n                <p>For our developer community, feel free to join our Google groups at:</p>\n\n                <ul>\n                    <li><a href=\"https://groups.google.com/forum/#!forum/vmware-lightwave-dev\">vmware-lightwave-dev</a></li>\n                    <li><a href=\"https://groups.google.com/forum/#!forum/vmware-lightwave-announce\">vmware-lightwave-announce</a></li>\n                </ul>\n                \n                <p>For more general user questions, visit the Lightwave&trade; user forum in our <a href=\"http://communities.vmware.com/community/vmtn/devops/project-lightwave\">Lightwave&trade; VMware Community</a>.</p>\n            </div>\n            <div id=\"contributors\" class=\"section\">\n                <h2>Contributors</h2>\n\n                <p>\n                    <a title=\"Krishna Ganugapati\" href=\"https://github.com/kganugapati\"><img alt=\"Krishna Ganugapati\" src=\"https://avatars.githubusercontent.com/u/11167611?v=3\" class=\"contributor\"></a>\n                    <a title=\"Sriram Nambakam\" href=\"https://github.com/snambakam\"><img alt=\"Sriram Nambakam\" src=\"https://avatars.githubusercontent.com/u/10917504?v=3\" class=\"contributor\"></a>\n                    <a title=\"Johnny Ferguson\" href=\"https://github.com/johnnyferguson\"><img alt=\"Johnny Ferguson\" src=\"https://avatars.githubusercontent.com/u/4875765?v=3\" class=\"contributor\"></a>\n                    <a title=\"aishu\" href=\"https://github.com/aishu\"><img alt=\"aishu\" src=\"https://avatars.githubusercontent.com/u/1206719?v=3\" class=\"contributor\"></a>\n                    <a title=\"Andrei Izurov\" href=\"https://github.com/aizurov-vmw\"><img alt=\"Andrei Izurov\" src=\"https://avatars.githubusercontent.com/u/11841394?v=3\" class=\"contributor\"></a>\n                    <a title=\"Balaji Boggaram Ramanarayan\" href=\"https://github.com/balajiboggaram\"><img alt=\"Balaji Boggaram Ramanarayan\" src=\"https://avatars.githubusercontent.com/u/7728418?v=3\" class=\"contributor\"></a>\n                    <a title=\"elenashutova\" href=\"https://github.com/elenashutova\"><img alt=\"elenashutova\" src=\"https://avatars.githubusercontent.com/u/12536702?v=3\" class=\"contributor\"></a>\n                    <a title=\"gsadhani\" href=\"https://github.com/gsadhani\"><img alt=\"gsadhani\" src=\"https://avatars.githubusercontent.com/u/11847110?v=3\" class=\"contributor\"></a>\n                    <a title=\"Jason Schroeder\" href=\"https://github.com/jasonsch\"><img alt=\"Jason Schroeder\" src=\"https://avatars.githubusercontent.com/u/1274313?v=3\" class=\"contributor\"></a>\n                    <a title=\"Adam Bernstein\" href=\"https://github.com/numberer6\"><img alt=\"Adam Bernstein\" src=\"https://avatars.githubusercontent.com/u/11415543?v=3\" class=\"contributor\"></a>\n                    <a title=\"Priyesh\" href=\"https://github.com/ppadmavilasom\"><img alt=\"Priyesh\" src=\"https://avatars.githubusercontent.com/u/11167452?v=3\" class=\"contributor\"></a>\n                    <a title=\"Jonathan Brown\" href=\"https://github.com/sabertail\"><img alt=\"Jonathan Brown\" src=\"https://avatars.githubusercontent.com/u/738054?v=3\" class=\"contributor\"></a>\n                    <a title=\"schellappan\" href=\"https://github.com/schellappan\"><img alt=\"schellappan\" src=\"https://avatars.githubusercontent.com/u/11465231?v=3\" class=\"contributor\"></a>\n                    <a title=\"shenoykrish\" href=\"https://github.com/shenoykrish\"><img alt=\"shenoykrish\" src=\"https://avatars.githubusercontent.com/u/11840667?v=3\" class=\"contributor\"></a>\n                    <a title=\"Scott Salley\" href=\"https://github.com/ssalley\"><img alt=\"Scott Salley\" src=\"https://avatars.githubusercontent.com/u/11413034?v=3\" class=\"contributor\"></a>\n                    <a title=\"Sumalatha Abhishek\" href=\"https://github.com/sumasekar\"><img alt=\"Sumalatha Abhishek\" src=\"https://avatars.githubusercontent.com/u/4172554?v=3\" class=\"contributor\"></a>\n                    <a title=\"Travis Hall\" href=\"https://github.com/tvs\"><img alt=\"Travis Hall\" src=\"https://avatars.githubusercontent.com/u/145500?v=3\" class=\"contributor\"></a>\n                    <a title=\"junsvmware\" href=\"https://github.com/junsvmware\"><img alt=\"junsvmware\" src=\"https://avatars.githubusercontent.com/u/15912571?v=3\" class=\"contributor\"></a>\n                    <a title=\"Nimish Bhonsale\" href=\"https://github.com/nimishbhonsale-vmware\"><img alt=\"Nimish Bhonsale\" src=\"https://avatars.githubusercontent.com/u/12462271?v=3\" class=\"contributor\"></a>\n                    <a title=\"wdu88\" href=\"https://github.com/wdu88\"><img alt=\"wdu88\" src=\"https://avatars.githubusercontent.com/u/12436217?v=3\" class=\"contributor\"></a>\n                    <a title=\"Kyoung Won Kwon\" href=\"https://github.com/kyoungkwon\"><img alt=\"Kyoung Won Kwon\" src=\"https://avatars.githubusercontent.com/u/13558419?v=3\" class=\"contributor\"></a>\n                    <a title=\"Anca Antochi\" href=\"https://github.com/ancaantochi\"><img alt=\"Anca Antochi\" src=\"https://avatars.githubusercontent.com/u/15937192?v=3\" class=\"contributor\"></a>\n                    <a title=\"Andrew Gormley\" href=\"https://github.com/agormley\"><img alt=\"Andrew Gormley\" src=\"https://avatars.githubusercontent.com/u/15987205?v=3\" class=\"contributor\"></a>\n                    <a title=\"rlohia\" href=\"https://github.com/rlohia\"><img alt=\"rlohia\" src=\"https://avatars.githubusercontent.com/u/11660743?v=3\" class=\"contributor\"></a>\n                    <a title=\"singhsaurabh005\" href=\"https://github.com/singhsaurabh005\"><img alt=\"singhsaurabh005\" src=\"https://avatars.githubusercontent.com/u/11926051?v=3\" class=\"contributor\"></a>\n                    <a title=\"harisso\" href=\"https://github.com/harisso\"><img alt=\"harisso\" src=\"https://avatars.githubusercontent.com/u/11852065?v=3\" class=\"contributor\"></a>\n                    <a title=\"sjchase\" href=\"https://github.com/sjchase\"><img alt=\"sjchase\" src=\"https://avatars.githubusercontent.com/u/1044976?v=3\" class=\"contributor\"></a>\n                    <a title=\"Divya Mehta\" href=\"https://github.com/divyamehta\"><img alt=\"divyamehta\" src=\"https://avatars3.githubusercontent.com/u/729513?v=3\" class=\"contributor\"></a>\n                </p>\n            </div>\n\n            <div id=\"contributing\" class=\"section\">\n                <h2>Contributing</h2>\n\n                <p>The Lightwave&trade; team will open the project to community contributions soon. Until then please feel free to fork the project and try out the RPMs and provide feedback in the support communities.</p>\n            </div>\n\n            <div id=\"license\" class=\"section\">\n                <h2>License</h2>\n\n                <p>Lightwave&trade; is comprised of many open source software components, each of which has its own license that is located in the source code of the respective component as well as documented in the <a href=\"https://github.com/vmware/lightwave/blob/master/open_source_license.txt\">open source license file</a> accompanying the Lightwave&trade; distribution.</p>\n            </div>\n        </div>\n        <nav class=\"sidenav\" [clr-nav-level]=\"2\">\n            <section class=\"sidenav-content\">\n                <section class=\"nav-group\" [scrollspy]=\"scrollable\">\n                    <label><a class=\"nav-link active\" routerLink=\".\" routerLinkActive=\"active\" fragment=\"overview\">Overview</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"gettingLightwave\">Getting Lightwave&trade;</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"gettingStarted\">Getting Started</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"support\">Support</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"contributors\">Contributors</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"contributing\">Contributing</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"license\">License</a></label>\n                </section>\n            </section>\n        </nav>\n    </div>\n</clr-main-container>\n"

/***/ }),

/***/ 360:
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__(142);


/***/ }),

/***/ 89:
/***/ (function(module, exports, __webpack_require__) {

"use strict";

var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
var __metadata = (this && this.__metadata) || function (k, v) {
    if (typeof Reflect === "object" && typeof Reflect.metadata === "function") return Reflect.metadata(k, v);
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = __webpack_require__(10);
var router_1 = __webpack_require__(47);
var AppComponent = (function () {
    function AppComponent(router) {
        this.router = router;
    }
    return AppComponent;
}());
AppComponent = __decorate([
    core_1.Component({
        selector: 'my-app',
        template: __webpack_require__(329),
        styles: [__webpack_require__(320)]
    }),
    __metadata("design:paramtypes", [typeof (_a = typeof router_1.Router !== "undefined" && router_1.Router) === "function" && _a || Object])
], AppComponent);
exports.AppComponent = AppComponent;
var _a;
//# sourceMappingURL=/Users/druk/Sites/lightwave/src/src/src/app/app.component.js.map

/***/ })

},[360]);
//# sourceMappingURL=main.bundle.js.map