webpackJsonp([1],{

/***/ "./src async recursive":
/***/ (function(module, exports) {

function webpackEmptyContext(req) {
	throw new Error("Cannot find module '" + req + "'.");
}
webpackEmptyContext.keys = function() { return []; };
webpackEmptyContext.resolve = webpackEmptyContext;
module.exports = webpackEmptyContext;
webpackEmptyContext.id = "./src async recursive";

/***/ }),

/***/ "./src/app/app.component.html":
/***/ (function(module, exports) {

module.exports = "<clr-main-container>\n    <header class=\"header header-6\">\n        <div class=\"branding\">\n            <a href=\"https://vmware.github.io/\" class=\"nav-link\">\n                <span class=\"clr-icon vmware-logo\"></span>\n                <span class=\"title\">Open Source Program Office</span>\n            </a>\n        </div>\n    </header>\n    <div class=\"content-container\">\n        <div id=\"content-area\" class=\"content-area\" hash-listener #scrollable>\n            <div class=\"hero\">\n                <div class=\"hero-image\"><img src=\"images/lightwave.png\" alt=\"VMware Lightwave&trade;\"></div>\n                <h3>Security for Container Ecosystem</h3>\n                <p><a href=\"https://github.com/vmware/lightwave\" class=\"btn btn-primary\"><i class=\"icon icon-github\"></i> Fork Lightwave&trade;</a></p>\n            </div>\n            <div id=\"overview\" class=\"section\">\n                <h2>What is Lightwave</h2>\n\n                <p>Project Lightwave is an open source project comprised of enterprise-grade, identity and access management services targeting critical security, governance, and compliance challenges for Cloud-Native Apps within the enterprise. Through integration with Project Photon, Project Lightwave can provide security and governance for container workloads. Project Lightwave can also serve a variety of use cases such as single sign-on, authentication, authorization and certificate authority, as well as certificate key management services across the entire infrastructure and application stack. Project Lightwave is based on a production quality code and an enterprise-grade architecture that is multi-tenant, scalable, and highly available multi master replication topology.</p>\n\n                <p>Project Lightwave is made up of the following key identity infrastructure elements:</p>\n\n                <br>\n\n                <ul>\n                    <li><strong>Lightwave Directory Service</strong> - standards based, multi-tenant, multi-master, highly scalable LDAP v3 directory service enables an enterprise’s infrastructure to be used by the most-demanding applications as well as by multiple teams.</li>\n                    <li><strong>Lightwave Certificate Authority</strong> - directory integrated certificate authority helps to simplify certificate-based operations and key management across the infrastructure.</li>\n                    <li><strong>Lightwave Certificate Store</strong> - endpoint certificate store to store certificate credentials.</li>\n                    <li><strong>Lightwave Authentication Services</strong> - cloud authentication services with support for Kerberos, OAuth 2.0/OpenID Connect, SAML and WSTrust enable interoperability with other standards-based technologies in the data center.</li>\n                </ul>\n            </div>\n\n            <div id=\"gettingLightwave\" class=\"section\">\n                <h2>Getting Lightwave&trade;</h2>\n\n                <p>You can download the latest Project Lightwave source code from <a href=\"https://github.com/vmware/lightwave\">Github</a>. Project Lightwave has been validated with Project Photon and should work with most Linux container run time hosts. You can also get Lightwave RPMs from <a href=\"https://bintray.com/vmware/lightwave\">Bintray</a>.</p>\n            </div>\n\n            <div id=\"gettingStarted\" class=\"section\">\n                <h2>Getting Started</h2>\n                \n                <p>We’ve provided a few guides to help get you started:</p>\n\n                <p>You can download the latest Project Lightwave source code and documentation including the <a href=\"https://github.com/vmware/lightwave/blob/master/README.md\">README</a>. Project Lightwave has been validated with Project Photon and should work with most Linux container run time hosts.</p>\n            </div>\n\n            <div id=\"support\" class=\"section\">\n                <h2>Support</h2>\n                \n                <p>Lightwave&trade; is released as open source software and comes with no commercial support.</p>\n\n                <p>But since we want to ensure success and recognize that Lightwave&trade; consumers might fall into a range of roles - from developers that are steeped in the conventions of open-source to customers that are more accustomed to VMware commercial offerings, we offer several methods of engaging with the Lightwave&trade; team and community.</p>\n\n                <p>For our developer community, feel free to join our Google groups at:</p>\n\n                <ul>\n                    <li><a href=\"https://groups.google.com/forum/#!forum/vmware-lightwave-dev\">vmware-lightwave-dev</a></li>\n                    <li><a href=\"https://groups.google.com/forum/#!forum/vmware-lightwave-announce\">vmware-lightwave-announce</a></li>\n                </ul>\n                \n                <p>For more general user questions, visit the Lightwave&trade; user forum in our <a href=\"http://communities.vmware.com/community/vmtn/devops/project-lightwave\">Lightwave&trade; VMware Community</a>.</p>\n            </div>\n            <div id=\"contributors\" class=\"section\">\n                <h2>Contributors</h2>\n\n                <br>\n\n                <ul class=\"list-unstyled row\">\n                    <li *ngFor=\"let contributor of contributors\"><a [href]=\"contributor.html_url\"><img [src]=\"contributor.avatar_url\" alt=\"\" class=\"contributor\"></a></li>\n                </ul>\n            </div>\n\n            <div id=\"contributing\" class=\"section\">\n                <h2>Contributing</h2>\n\n                <p>The Lightwave&trade; team will open the project to community contributions soon. Until then please feel free to fork the project and try out the RPMs and provide feedback in the support communities.</p>\n            </div>\n\n            <div id=\"license\" class=\"section\">\n                <h2>License</h2>\n\n                <p>Lightwave&trade; is comprised of many open source software components, each of which has its own license that is located in the source code of the respective component as well as documented in the <a href=\"https://github.com/vmware/lightwave/blob/master/open_source_license.txt\">open source license file</a> accompanying the Lightwave&trade; distribution.</p>\n            </div>\n        </div>\n        <nav class=\"sidenav\" [clr-nav-level]=\"2\">\n            <section class=\"sidenav-content\">\n                <section class=\"nav-group\" [scrollspy]=\"scrollable\">\n                    <label><a class=\"nav-link active\" routerLink=\".\" routerLinkActive=\"active\" fragment=\"overview\">Overview</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"gettingLightwave\">Getting Lightwave&trade;</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"gettingStarted\">Getting Started</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"support\">Support</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"contributors\">Contributors</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"contributing\">Contributing</a></label>\n                    <label class=\"bump-down\"><a class=\"nav-link\" routerLink=\".\" fragment=\"license\">License</a></label>\n                </section>\n            </section>\n        </nav>\n    </div>\n</clr-main-container>\n"

/***/ }),

/***/ "./src/app/app.component.scss":
/***/ (function(module, exports, __webpack_require__) {

exports = module.exports = __webpack_require__("./node_modules/css-loader/lib/css-base.js")(false);
// imports


// module
exports.push([module.i, ".clr-icon.vmware-logo {\n  background: url(/lightwave/images/vmware.svg) no-repeat left 9px;\n  width: 108px; }\n\n.hero {\n  background-color: #ddd;\n  left: -24px;\n  padding-bottom: 2em;\n  padding-top: 2em;\n  overflow-x: hidden;\n  position: relative;\n  text-align: center;\n  top: -24px; }\n  .hero .btn-custom {\n    display: inline-block;\n    text-align: center;\n    margin: auto; }\n\n.content-area {\n  overflow-x: hidden; }\n\n.hero-image img {\n  max-width: 360px; }\n\n.icon {\n  display: inline-block;\n  height: 32px;\n  vertical-align: middle;\n  width: 32px; }\n  .icon.icon-github {\n    background: url(/lightwave/images/github_icon.svg) no-repeat left -2px; }\n\n.nav-group label {\n  display: block;\n  margin-bottom: 1em; }\n\n.sidenav .nav-link {\n  padding: 3px 6px; }\n  .sidenav .nav-link:hover {\n    background: #eee; }\n  .sidenav .nav-link.active {\n    background: #d9e4ea;\n    color: #000; }\n\n.section {\n  padding: .5em 0; }\n\n.contributor {\n  border-radius: 50%;\n  border: 1px solid #ccc;\n  margin-bottom: 1.5em;\n  margin-right: 1em;\n  max-width: 64px;\n  text-decoration: none; }\n\n@media (min-width: 320px) {\n  .title {\n    display: none; }\n  .hero {\n    width: 100vw; }\n  #license {\n    padding-bottom: 20vh; } }\n\n@media (min-width: 768px) {\n  .title {\n    display: block; }\n  .hero {\n    width: 110%; }\n  #license {\n    padding-bottom: 78vh; } }\n\n.row:after {\n  clear: both;\n  content: \"\";\n  display: table; }\n", ""]);

// exports


/*** EXPORTS FROM exports-loader ***/
module.exports = module.exports.toString();

/***/ }),

/***/ "./src/app/app.component.ts":
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
var core_1 = __webpack_require__("./node_modules/@angular/core/@angular/core.es5.js");
var contributors_service_1 = __webpack_require__("./src/services/contributors.service.ts");
var AppComponent = (function () {
    function AppComponent(contributorSvc) {
        this.contributorSvc = contributorSvc;
        this.contributors = [];
    }
    AppComponent.prototype.ngOnInit = function () {
        var _this = this;
        this.contributorSvc.getContributors().subscribe(function (results) {
            _this.contributors = results;
            // console.log("Contribs: ", results);
        });
    };
    return AppComponent;
}());
AppComponent = __decorate([
    core_1.Component({
        selector: 'my-app',
        template: __webpack_require__("./src/app/app.component.html"),
        styles: [__webpack_require__("./src/app/app.component.scss")]
    }),
    __metadata("design:paramtypes", [typeof (_a = typeof contributors_service_1.ContributorService !== "undefined" && contributors_service_1.ContributorService) === "function" && _a || Object])
], AppComponent);
exports.AppComponent = AppComponent;
var _a;
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/app/app.component.js.map

/***/ }),

/***/ "./src/app/app.module.ts":
/***/ (function(module, exports, __webpack_require__) {

"use strict";

var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
Object.defineProperty(exports, "__esModule", { value: true });
var platform_browser_1 = __webpack_require__("./node_modules/@angular/platform-browser/@angular/platform-browser.es5.js");
var core_1 = __webpack_require__("./node_modules/@angular/core/@angular/core.es5.js");
var forms_1 = __webpack_require__("./node_modules/@angular/forms/@angular/forms.es5.js");
var http_1 = __webpack_require__("./node_modules/@angular/http/@angular/http.es5.js");
var clarity_angular_1 = __webpack_require__("./node_modules/clarity-angular/index.js");
var app_component_1 = __webpack_require__("./src/app/app.component.ts");
var utils_module_1 = __webpack_require__("./src/utils/utils.module.ts");
var app_routing_1 = __webpack_require__("./src/app/app.routing.ts");
var contributors_service_1 = __webpack_require__("./src/services/contributors.service.ts");
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
        providers: [contributors_service_1.ContributorService],
        bootstrap: [app_component_1.AppComponent]
    })
], AppModule);
exports.AppModule = AppModule;
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/app/app.module.js.map

/***/ }),

/***/ "./src/app/app.routing.ts":
/***/ (function(module, exports, __webpack_require__) {

"use strict";

Object.defineProperty(exports, "__esModule", { value: true });
var router_1 = __webpack_require__("./node_modules/@angular/router/@angular/router.es5.js");
exports.ROUTES = [
    { path: '', redirectTo: 'home', pathMatch: 'full' }
];
exports.ROUTING = router_1.RouterModule.forRoot(exports.ROUTES);
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/app/app.routing.js.map

/***/ }),

/***/ "./src/app/index.ts":
/***/ (function(module, exports, __webpack_require__) {

"use strict";

function __export(m) {
    for (var p in m) if (!exports.hasOwnProperty(p)) exports[p] = m[p];
}
Object.defineProperty(exports, "__esModule", { value: true });
__export(__webpack_require__("./src/app/app.component.ts"));
__export(__webpack_require__("./src/app/app.module.ts"));
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/app/index.js.map

/***/ }),

/***/ "./src/environments/environment.ts":
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
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/environments/environment.js.map

/***/ }),

/***/ "./src/main.ts":
/***/ (function(module, exports, __webpack_require__) {

"use strict";

Object.defineProperty(exports, "__esModule", { value: true });
__webpack_require__("./src/polyfills.ts");
var platform_browser_dynamic_1 = __webpack_require__("./node_modules/@angular/platform-browser-dynamic/@angular/platform-browser-dynamic.es5.js");
var core_1 = __webpack_require__("./node_modules/@angular/core/@angular/core.es5.js");
var environment_1 = __webpack_require__("./src/environments/environment.ts");
var _1 = __webpack_require__("./src/app/index.ts");
if (environment_1.environment.production) {
    core_1.enableProdMode();
}
platform_browser_dynamic_1.platformBrowserDynamic().bootstrapModule(_1.AppModule);
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/main.js.map

/***/ }),

/***/ "./src/polyfills.ts":
/***/ (function(module, exports, __webpack_require__) {

"use strict";

Object.defineProperty(exports, "__esModule", { value: true });
// This file includes polyfills needed by Angular 2 and is loaded before
// the app. You can add your own extra polyfills to this file.
__webpack_require__("./node_modules/core-js/es6/symbol.js");
__webpack_require__("./node_modules/core-js/es6/object.js");
__webpack_require__("./node_modules/core-js/es6/function.js");
__webpack_require__("./node_modules/core-js/es6/parse-int.js");
__webpack_require__("./node_modules/core-js/es6/parse-float.js");
__webpack_require__("./node_modules/core-js/es6/number.js");
__webpack_require__("./node_modules/core-js/es6/math.js");
__webpack_require__("./node_modules/core-js/es6/string.js");
__webpack_require__("./node_modules/core-js/es6/date.js");
__webpack_require__("./node_modules/core-js/es6/array.js");
__webpack_require__("./node_modules/core-js/es6/regexp.js");
__webpack_require__("./node_modules/core-js/es6/map.js");
__webpack_require__("./node_modules/core-js/es6/set.js");
__webpack_require__("./node_modules/core-js/es6/reflect.js");
__webpack_require__("./node_modules/core-js/es7/reflect.js");
__webpack_require__("./node_modules/zone.js/dist/zone.js");
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/polyfills.js.map

/***/ }),

/***/ "./src/services/contributors.service.ts":
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
var core_1 = __webpack_require__("./node_modules/@angular/core/@angular/core.es5.js");
var http_1 = __webpack_require__("./node_modules/@angular/http/@angular/http.es5.js");
var Observable_1 = __webpack_require__("./node_modules/rxjs/Observable.js");
__webpack_require__("./node_modules/rxjs/add/observable/forkJoin.js");
__webpack_require__("./node_modules/rxjs/add/operator/map.js");
var ContributorService = (function () {
    function ContributorService(http) {
        this.http = http;
    }
    ContributorService.prototype.getContributors = function () {
        // do work to merge three http calls into one observable.
        return Observable_1.Observable.forkJoin([
            this.http.get('https://api.github.com/repos/vmware/lightwave/contributors')
                .map(function (res) { return res.json(); })
        ])
            .map(function (data) {
            var contributors = [];
            // console.logco("observable data", data); // make sure we are getting datas from github.
            // concat all the data into one array
            contributors = contributors.concat(data[0]);
            // create a uniqueContributors array
            var uniqueContributors = [];
            // filteredContributors filters contributors array, add it to uniqueContributors if its not already there.
            var filteredContributors = contributors.filter(function (el) {
                if (uniqueContributors.indexOf(el.id) === -1) {
                    uniqueContributors.push(el.id);
                    return true;
                }
                else {
                    return false;
                }
            });
            contributors = filteredContributors;
            return contributors;
        });
    };
    return ContributorService;
}());
ContributorService = __decorate([
    core_1.Injectable(),
    __metadata("design:paramtypes", [typeof (_a = typeof http_1.Http !== "undefined" && http_1.Http) === "function" && _a || Object])
], ContributorService);
exports.ContributorService = ContributorService;
var _a;
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/services/contributors.service.js.map

/***/ }),

/***/ "./src/utils/hash-listener.directive.ts":
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
var core_1 = __webpack_require__("./node_modules/@angular/core/@angular/core.es5.js");
var router_1 = __webpack_require__("./node_modules/@angular/router/@angular/router.es5.js");
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
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/utils/hash-listener.directive.js.map

/***/ }),

/***/ "./src/utils/scrollspy.directive.ts":
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
var core_1 = __webpack_require__("./node_modules/@angular/core/@angular/core.es5.js");
var router_1 = __webpack_require__("./node_modules/@angular/router/@angular/router.es5.js");
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
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/utils/scrollspy.directive.js.map

/***/ }),

/***/ "./src/utils/utils.module.ts":
/***/ (function(module, exports, __webpack_require__) {

"use strict";

var __decorate = (this && this.__decorate) || function (decorators, target, key, desc) {
    var c = arguments.length, r = c < 3 ? target : desc === null ? desc = Object.getOwnPropertyDescriptor(target, key) : desc, d;
    if (typeof Reflect === "object" && typeof Reflect.decorate === "function") r = Reflect.decorate(decorators, target, key, desc);
    else for (var i = decorators.length - 1; i >= 0; i--) if (d = decorators[i]) r = (c < 3 ? d(r) : c > 3 ? d(target, key, r) : d(target, key)) || r;
    return c > 3 && r && Object.defineProperty(target, key, r), r;
};
Object.defineProperty(exports, "__esModule", { value: true });
var core_1 = __webpack_require__("./node_modules/@angular/core/@angular/core.es5.js");
var hash_listener_directive_1 = __webpack_require__("./src/utils/hash-listener.directive.ts");
var scrollspy_directive_1 = __webpack_require__("./src/utils/scrollspy.directive.ts");
var clarity_angular_1 = __webpack_require__("./node_modules/clarity-angular/index.js");
var common_1 = __webpack_require__("./node_modules/@angular/common/@angular/common.es5.js");
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
//# sourceMappingURL=/Users/druk/ID&D Assets/Open Source Software/lightwave/src/src/src/utils/utils.module.js.map

/***/ }),

/***/ 1:
/***/ (function(module, exports, __webpack_require__) {

module.exports = __webpack_require__("./src/main.ts");


/***/ })

},[1]);
//# sourceMappingURL=main.bundle.js.map