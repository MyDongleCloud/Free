import { Injectable } from '@angular/core';
import { CanActivate, ActivatedRouteSnapshot, RouterStateSnapshot, Router } from '@angular/router';
import { HttpClient } from '@angular/common/http';
import { Platform } from '@ionic/angular';
import { NavController, AlertController, MenuController } from '@ionic/angular';
import { PopoverController } from '@ionic/angular';
import { Filesystem, Directory } from '@capacitor/filesystem';
import { App } from '@capacitor/app';
import { Device } from '@capacitor/device';
import { Preferences } from '@capacitor/preferences';
import { InAppReview } from '@capacitor-community/in-app-review';
import { TranslateService } from '@ngx-translate/core';
import { Subject } from 'rxjs';
import { environment } from '../environments/environment';
import FingerprintJS from '@fingerprintjs/fingerprintjs';
import { Settings, CategoriesEx } from './myinterface';
import { VERSION } from './version';
import modulesDefault from './modulesdefault.json';
import modulesMeta from './modulesmeta.json';
//import { toASCII } from 'punycode';
import * as ACME from '@root/acme';
import * as Keypairs from '@root/keypairs';
import * as CSR from '@root/csr';
import * as PEM from '@root/pem';
import * as ENC from '@root/encoding/base64';

declare var appConnectToggle: any;

@Injectable({
	providedIn: "root"
})

export class Global implements CanActivate {
developer: boolean = false;
demo: boolean = false;
splashDone = false;
VERSION: string = VERSION;
SERVERURL: string = "https://mydongle.cloud";
currentUrl: string;
activateUrl: string;
settings: Settings = { lang:"en", welcomeTourShown:false } as Settings;
refreshUI:Subject<any> = new Subject();
toast:Subject<any> = new Subject();
firmwareServerVersion;
session;
modulesData = [];
sidebarFilterType = "";
sidebarSearchTerm = "";
statsIntervalId;
statsPeriod;
statsData;
themeSel = "system";
darkVal = false;
setupUIProgress = 0;
setupUIDesc = "";

constructor(public plt: Platform, private router: Router, private navCtrl: NavController, private alertCtrl: AlertController, private menu: MenuController, private translate: TranslateService, public popoverController: PopoverController, private httpClient: HttpClient) {
	this.developer = this.developerGet();
	this.consolelog(0, "%c⛅ MyDongle.Cloud: my data, my cloud, my sovereignty 🚀", "font-weight:bold; font-size:x-large;");
	this.consolelog(0, "%cDocs: https://docs.mydongle.cloud", "font-weight:bold; font-size:large;");
	this.consolelog(0, "%cVersion: " + this.VERSION, "background-color:#646464; border-radius:5px; padding:5px;");
	this.consolelog(0, "%cPlease give a ⭐ to this project at:", "color:black; background-color:#fef9c2; border-radius:5px; padding:5px;");
	this.consolelog(0, "%chttps://github.com/mydonglecloud/free", "border:1px solid white; border-radius:5px; padding:5px; font-weight:bold;");
	this.consolelog(1, "Platform: " + this.plt.platforms());
	navCtrl.setDirection("forward");
	translate.setDefaultLang("en");
	this.consolelog(1, "Default browser language: " + translate.getBrowserLang());
	if (window.location.hostname.indexOf("mondongle.cloud") != -1)
		this.changeLanguage("fr");
	else
		this.changeLanguage(this.translate.getBrowserLang());
	this.AuthStatus();
	this.getSession();
	window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => { this.themeSet(); });
	this.themeSet();
	this.getVisitorHash();
}

consolelog(level, ...st) {
	if (level == 0 || this.developer)
		console.log(...st);
}

themeSet(t = null) {
	if (t !== null)
		this.themeSel = t;
	if (this.themeSel == "system")
		this.darkVal = window.matchMedia("(prefers-color-scheme: dark)").matches;
	else if (this.themeSel == "dark")
		this.darkVal = true;
	else
		this.darkVal = false;
	const darkCurrent = document.body.classList.contains("dark");
	if (darkCurrent == false && this.darkVal == true)
		document.body.classList.add("dark");
	if (darkCurrent == true && this.darkVal == false)
		document.body.classList.remove("dark");
}

async getVisitorHash() {
	const fp = await FingerprintJS.load();
	const result = await fp.get();
	const visitorId = result.visitorId;
	this.consolelog(1, "Fingerprint: " + visitorId);
	return visitorId;
}

getCookie(name) {
	const allCookies = document.cookie;
	const nameEQ = name + "=";
	const ca = document.cookie.split(';');
	for(let i=0; i < ca.length; i++) {
		let c = ca[i];
		while (c.charAt(0) === ' ') c = c.substring(1, c.length);
		if (c.indexOf(nameEQ) === 0)
			return decodeURIComponent(c.substring(nameEQ.length, c.length));
	}
	return null;
}

domainFromFqdn(fqdn) {
	const parts = fqdn.split('.');
	if (parts.length <= 2)
		return fqdn;
	if (!isNaN(parts[parts.length - 1]))
		return fqdn;
	const sliceIndex = (parts[parts.length - 2] === "mydongle" || parts[parts.length - 2] === "mondongle" || parts[parts.length - 2] === "myd") ? -3 : -2;
	return parts.slice(sliceIndex).join('.');
}

setCookie(name, value, domain = null) {
	const host = window.location.hostname.replace(/^([^:]*)(?::\d+)?$/i, '$1');
	if (domain == null)
		domain = this.domainFromFqdn(host);
	if (value == "")
		document.cookie = `${name}=; Domain=${domain}; Path=/; Expires=Thu, 01 Jan 1970 00:00:01 GMT;`;
	else
		document.cookie = `${name}=${value}; Domain=${domain}; Path=/;`;
}

async AuthStatus() {
	const ret = await this.httpClient.get("/_app_/auth/status", {headers:{"content-type": "application/json"}}).toPromise();
	if (ret["demo"] === true)
		this.demo = true;
	this.consolelog(2, "Auth Status: ", ret);
}

developerSet(a = null) {
	if (a === null)
		this.developer = !this.developer;
	else
		this.developer = a;
	sessionStorage.setItem("developer", String(this.developer));
}

developerGet() {
	if (sessionStorage.getItem("developer") === null) {
		const val = window.location.hostname == "localhost" && window.location.port == "8100";
		sessionStorage.setItem("developer", String(val));
		return val;
	} else
		return sessionStorage.getItem("developer") === "true";
}

async getSession() {
	this.session = await this.httpClient.get("/_app_/auth/get-session", {headers:{"content-type": "application/json"}}).toPromise();
	this.consolelog(2, "Auth get-session: ", this.session);
	if (this.session != null) {
		const jwt = await this.httpClient.get("/_app_/auth/token", {headers:{"content-type": "application/json"}}).toPromise();
		this.settings = JSON.parse(this.session.user.settings);
		await this.translate.use(this.settings.lang);
		await this.modulesDataPrepare();
		this.statsPeriod = this.developer ? 1 : this.session.user.role == "admin" ? 10 : 30;
		this.statsStartPolling();
	}
}

async logout() {
	this.setCookie("jwt", "");
	const data = { token:this.session?.session?.token };
	try {
		const ret = await this.httpClient.post("/_app_/auth/revoke-session", JSON.stringify(data), {headers:{"content-type": "application/json"}}).toPromise();
		this.consolelog(2, "Auth revoke-session: ", ret);
	} catch(e) {}
	this.session = null;
}

async logoutRedirect() {
	await this.logout();
	this.consolelog(1, "logout");
	document.location.href = "/";
}

async settingsSave() {
	try {
		const ret = await this.httpClient.post("/_app_/auth/settings-user/save", JSON.stringify(this.settings), {headers:{"content-type": "application/json"}}).toPromise();
		this.consolelog(2, "Auth settings-user/save: ", ret);
	} catch(e) {}
}

async backButtonAlert() {
	const alert = await this.alertCtrl.create({
		message: this.mytranslateP("splash", "Do you want to leave this application?"),
		buttons: [{
			text: "cancel",
			role: "cancel"
		},{
			text: "Close App",
			handler: () => {
				App.exitApp();
			}
		}]
	});
	await alert.present();
}

async presentQuestion(hd, st, msg, key:string = "") {
	if (this.settings.dontShowAgain?.[key] !== undefined)
		return false;
	let checked = false;
	let yesClicked = false;
	const question = await this.alertCtrl.create({
		cssClass: "basic-alert",
		header: hd,
		subHeader: st,
		message: msg,
		buttons: [{
			text: "Yes", handler: (data) => { yesClicked = true; if (data !== undefined && data.length > 0 && data[0]) checked = true; }
		}, {
			text: "No", cssClass: "secondary", handler: (data) => { yesClicked = false; if (data !== undefined && data.length > 0 && data[0]) checked = true; }
		}],
		inputs: key != "" ? [{ label:"Don't show again", type:"checkbox", checked:false, value:true }] : []
	});
	await question.present();
	await question.onDidDismiss();
	if (checked) {
		this.settings.dontShowAgain[key] = true;
		this.settingsSave();
	}
	return yesClicked;
}

async changeLanguage(st) {
	if (st != this.settings.lang) {
		this.settings.lang = st;
		await this.translate.use(this.settings.lang);
	}
}

mytranslateP(page, st) {
	const inp = page + "." + st;
	const ret = this.translate.instant(page + "." + st);
	return ret == "" ? (this.developer && this.settings.lang != "en" ? ("##" + st + "##") : st) : ret == inp ? (this.developer ? ("##" + st + "##") : st) : ret;
}

mytranslate(st) {
	return this.mytranslateP(this.currentUrl, st);
}

mytranslateG(st) {
	return this.mytranslateP("global", st);
}

mytranslateK(st) {
	return this.mytranslateP("keywords", st);
}

mytranslateMT(st) {
	return this.mytranslateP("modules.title", st);
}

mytranslateMD(st) {
	return this.mytranslateP("modules.description", st);
}

async sleepms(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

openBrowser(url: string) {
	window.open(url, "_blank");
}

openPage(url: string) {
	this.navCtrl.setDirection('root');
	const [path, queryString] = url.split("?");
	const params = new URLSearchParams(queryString);
	const obj = {};
	params.forEach((value, key) => { obj[key] = value; });
	this.router.navigate([path], { queryParams:obj });
}

openModule(identifier:number|string, extract:boolean = false, page:string = null) {
	let id = identifier;
	if (typeof identifier == "string")
		id = this.modulesDataFindId(identifier);
	if (this.modulesData[id].notReady != 0 && !this.demo) {
		this.presentToast("This module setup is under progress. It should be ready shortly...", "close-outline", 5000);
		return;
	}
	const subdomain = this.modulesData[id].alias[0] ?? this.modulesData[id].module;
	const page_ = page ?? this.modulesData[id].homepage ?? "";
	if (extract && !this.demo && (this.modulesData[id].finished || this.developer))
		window.open(location.protocol + "//" + location.host + "/m/" + subdomain + page_, "_blank");
	else {
		this.navCtrl.setDirection('root');
		this.router.navigate(["/wrapper"], { queryParams:{ module:this.modulesData[id].module, subdomain, page:page_ } });
	}
}

async presentAlert(hd, st, msg, key:string = "") {
	let checked = false;
	if (this.settings.dontShowAgain?.[key] !== undefined)
		return;
	const alert = await this.alertCtrl.create({
		cssClass: "basic-alert",
		header: hd,
		subHeader: st,
		message: msg,
		buttons: [{ text:"OK", handler: data => { if (data !== undefined && data.length > 0 && data[0]) checked = true; } }],
		inputs: key != "" ? [{label:"Don't show again", type:"checkbox", checked:false, value:true}] : []
	});
	await alert.present();
	await alert.onDidDismiss();
	if (checked) {
		this.settings.dontShowAgain[key] = true;
		this.settingsSave();
	}
}

dismissToast() {
	this.toast.next({ show:false });
}

presentToast(message, icon, delay = 3000) {
	this.toast.next({ show:true, message, icon, delay });
}

async platform() {
	if (this.plt.is("ios") && this.plt.is("cordova"))
		return "ios";
	else if (this.plt.is("android") && this.plt.is("cordova"))
		return "android";
	else
		return "web";
}

isPlatform(a) {
	if (a == "androidios")
		return (this.plt.is("android") || this.plt.is("ios")) && this.plt.is("cordova");
	else if (a == "android")
		return this.plt.is("android") && this.plt.is("cordova");
	else if (a == "ios")
		return this.plt.is("ios") && this.plt.is("cordova");
	else if (a == "web")
		return !((this.plt.is("android") && this.plt.is("cordova")) || (this.plt.is("ios") && this.plt.is("cordova")));
	else
		return false;
}

getMonth(a) {
	if (typeof a == "undefined")
		return "";
	else if (a.indexOf("01.01") != -1 || a.indexOf("01-01") != -1)
		return "January";
	else if (a.indexOf("02.02") != -1 || a.indexOf("02-02") != -1)
		return "February";
	else if (a.indexOf("03.03") != -1 || a.indexOf("03-03") != -1)
		return "March";
	else if (a.indexOf("04.04") != -1 || a.indexOf("04-04") != -1)
		return "April";
	else if (a.indexOf("05.05") != -1 || a.indexOf("05-05") != -1)
		return "May";
	else if (a.indexOf("06.06") != -1 || a.indexOf("06-06") != -1)
		return "June";
	else if (a.indexOf("07.07") != -1 || a.indexOf("07-07") != -1)
		return "July";
	else if (a.indexOf("08.08") != -1 || a.indexOf("08-08") != -1)
		return "August";
	else if (a.indexOf("09.09") != -1 || a.indexOf("09-09") != -1)
		return "September";
	else if (a.indexOf("10.10") != -1 || a.indexOf("10-10") != -1)
		return "October";
	else if (a.indexOf("11.11") != -1 || a.indexOf("11-11") != -1)
		return "November";
	else if (a.indexOf("12.12") != -1 || a.indexOf("12-12") != -1)
		return "December";
	return "";
}

permissions(st) {
	if (st == "_public_")
		return "Public";
	else if (st == "_localnetwork_")
		return "Local Network";
	else if (st == "_dongle_")
		return "Dongle";
	else if (st == "_groupadmin_" || st == "admin")
		return "Group Admin";
	else if (st == "_groupuser_" || st == "users")
		return "Group User";
	else
		return null;
}

colorWord(st, bckgd = true) {
	if (st == "_disabled_")
		return (bckgd ? "bg--red-100 " : "") + "text--red-800";

	if (st == "_public_")
		return (bckgd ? "bg--green-100 " : "") + "text--green-800";
	if (st == "_localnetwork_")
		return (bckgd ? "bg--yellow-100 " : "") + "text--yellow-800";
	if (st == "_groupadmin_" || st == "admin")
		return (bckgd ? "bg--yellow-100 " : "") + "text--yellow-800";
	if (st == "_groupuser_" || st == "users")
		return (bckgd ? "bg--purple-100 " : "") + "text--purple-800";

	if (CategoriesEx[st])
		return (bckgd ? "bg--" + CategoriesEx[st].color + "-100 " : "") + "text--" + CategoriesEx[st].color + "-600";
/*
bg--yellow-100 text--yellow-600
bg--purple-100 text--purple-600
bg--blue-100 text--blue-600
bg--cyan-100 text--cyan-600
bg--orange-100 text--orange-600
*/
	return (bckgd ? "bg--gray-100 " : "") + "text--gray-600";
}
 

colorWord2(st) {
	if (CategoriesEx[st])
		return "translate-y-[-256px] drop-shadow-[0px_256px_0_var(--color--" + CategoriesEx[st].color + "-600)]";
/*
translate-y-[-256px] drop-shadow-[0px_256px_0_var(--color--yellow-600)]
translate-y-[-256px] drop-shadow-[0px_256px_0_var(--color--purple-600)]
translate-y-[-256px] drop-shadow-[0px_256px_0_var(--color--blue-600)]
translate-y-[-256px] drop-shadow-[0px_256px_0_var(--color--cyan-600)]
translate-y-[-256px] drop-shadow-[0px_256px_0_var(--color--orange-600)]
*/
	return "translate-y-[-256px] drop-shadow-[0px_256px_0_var(--color--gray-600)]";
}

colorPercent(p, limits = [ 80, 50 ]) {
	if (p >= limits[0])
		return "bg--red-400";
	else if (p >= limits[1])
		return "bg--yellow-400";
	else
		return "bg--green-400";
}

formatCount(count) {
	if (count >= 1_000_000) {
		const rounded = (count / 1_000_000).toFixed(2);
		return rounded + "M";
	} else if (count >= 1000) {
		const rounded = (count / 1000).toFixed(1);
		return rounded + "k";
	} else
		return "" + count;
}

async modulesDataPrepare() {
	const modules = await this.httpClient.get("/_app_/auth/modules/data").toPromise();
	this.modulesData.length = 0;
	Object.entries(modulesMeta).forEach(([key, value]) => {
		if (modulesDefault[key] === undefined)
			this.consolelog(1, "Error: " + key + " not in modulesdefault");
		else if ((modulesDefault[key]["web"] === true && modulesMeta[key]["web"] !== true) || (modulesDefault[key]["web"] !== true && modulesMeta[key]["web"] === true))
			this.consolelog(1, "Error: " + key + " discrepancy web flag");
	});
	Object.entries(modulesDefault).forEach(([key, value]) => {
		if (modulesMeta[key] === undefined) {
			this.consolelog(1, "Error: " + key + " not in modulesmeta");
			return;
		}
		if (value["reservedToFirstUser"] === true && this.session.user.username != this.session.cloud.info.name)
			return;
		if (this.session.user.role == "user" && !value["web"])
			return;
		value["enabled"] = modules[key]?.enabled ?? value["enabled"] ?? true;
		value["notReady"] = modules[key]?.["setupDone"] !== true && value["setup"] === true && !this.demo ? 2 : 0;
		value["permissions"] = modules[key]?.permissions ?? value["permissions"];
		if (value["web"] !== true) {
			value["permissions"] = ["_groupadmin_"];
			value["web"] = false;
			value["enabled"] = true;
		}
		value["alias"] = [...(value["alias"] ?? []), ...(modules[key]?.alias ?? [])];
		if (value["web"]) {
			const ll = value["alias"].length > 0 ? value["alias"][0] : key;
			value["link"] = location.protocol + "//" + location.host + "/m/" + ll;
			value["link2"] = "https://" + ll + "." + (this.session?.["cloud"]?.["info"]?.["name"] ?? "") + ".mydongle.cloud";
			value["link"] = value["link"].toLowerCase();
			if (value["homepage"])
				value["link"] += value["homepage"];
			value["link2"] = value["link2"].toLowerCase();
		}
		Object.entries(modulesMeta[key]).forEach(([key2, value2]) => {
			value[key2] = value2;
		});
		value["tag"] = this.settings.tags?.includes(key)?? false;
		const items = [
			value["module"],
			value["name"],
			value["title"],
			value["category"],
			...value["keywords"],
			...value["proprietary"],
			...value["alias"],
		].filter(item => item !== "").map(v => v.toLowerCase());
		value["hayStack"] = [...new Set(items)];
		this.modulesData.push(value);
	});
	this.refreshUI.next("modules");
	if (!this.demo) {
		let setupDoneCount = 0;
		Object.entries(modules).forEach(([key, value]) => {
			if (value["setupDone"] === true)
				setupDoneCount++;
		});
		if (setupDoneCount < 44 && this.setupUIProgress == 0) {
			appConnectToggle(true);
			this.presentToast("First-time setup is under progress...", "help-outline", 0);
			this.setupUIDesc = "initialization";
			this.setupUIProgress = 1;
		}
	}
}

modulesDataFindId(m) {
	let ret = 0;
	this.modulesData.forEach((data, index) => {
		if (data["module"] == m)
			ret = index;
	});
	return ret;
}

review() {
	//this.settings.reviewRequestLastTime = Date.now();
	this.settingsSave();
	InAppReview.requestReview();
}

async statusRefresh(data) {
	if (data.progress)
		this.setupUIProgress = data.progress;
	if (data.module === "_setup_" && data.state === "finish") {
		appConnectToggle(false);
		this.setupUIProgress = 0;
		this.setupUIDesc = "";
		this.modulesData.forEach((data) => { data["notReady"] = 0; });
		this.presentToast("First-time setup is now complete!", "help-outline", 0);
	} else if (data.module && data.state === "start") {
		this.setupUIDesc = "module: " + data.module;
		this.modulesData[this.modulesDataFindId(data.module)]["notReady"] = 1;
	} else if (data.module && data.state === "finish")
		this.modulesData[this.modulesDataFindId(data.module)]["notReady"] = 0;
	this.refreshUI.next("refresh");
}

async statsPolling() {
	this.statsData = await this.httpClient.get("/_app_/auth/stats", {headers:{"content-type": "application/json"}}).toPromise();
	this.refreshUI.next("onlySidebar");
}

statsPeriodChange(incDir) {
	if (incDir > 0) {
		if (this.statsPeriod >= 10)
			this.statsPeriod = 30;
		else if (this.statsPeriod >= 5)
			this.statsPeriod = 10;
		else if (this.statsPeriod >= 1)
			this.statsPeriod = 5;
		else
			this.statsPeriod = 1;
	} else {
		if (this.statsPeriod >= 30)
			this.statsPeriod = 10;
		else if (this.statsPeriod >= 10)
			this.statsPeriod = 5;
		else if (this.statsPeriod >= 5)
			this.statsPeriod = 1;
	}
	this.statsStopPolling();
	this.statsStartPolling();
}

statsStartPolling() {
	this.statsPolling();
	this.statsIntervalId = setInterval(async () => { this.statsPolling(); }, 1000 * this.statsPeriod);
}

statsStopPolling() {
	if (this.statsIntervalId)
		clearInterval(this.statsIntervalId);
}

async getCertificate(name, shortname, additionalDomains) {
	const ret = { accountKey:"", accountKeyId:"", fullChain:"", privateKey:"" };
	const DOMAINNAME = ["mydongle.cloud", "mondongle.cloud", "myd.cd"];//.concat(additionalDomains);
	const STAGING = true;
	let acme = ACME.create({ maintainerEmail:"acme@@mydongle.cloud", packageAgent:"MDC/2025-01-01", notify:function (ev, msg) { /*this.consolelog(1, msg);*/ }, skipDryRun:true/*, skipChallengeTest:true*/ });
	await acme.init("https://acme" + (STAGING ? "-staging" : "") + "-v02.api.letsencrypt.org/directory");

	const accountKeypair = await Keypairs.generate({ kty: "EC", format: "jwk" });
	const accountKey = accountKeypair.private;
	ret.accountKey = await Keypairs.export({ jwk: accountKey });
	this.consolelog(2, "ACME: Registering account...");
	const account = await acme.accounts.create({ subscriberEmail:"certificate@mydongle.cloud", agreeToTerms:true, accountKey:accountKey });
	this.consolelog(2, "ACME: Created account with id ", account.key.kid);
	ret.accountKeyId = account.key.kid;

	const serverKeypair = await Keypairs.generate({ kty: "RSA", format: "jwk" });
	const serverKey = serverKeypair.private;
	ret.privateKey = await Keypairs.export({ jwk: serverKey });

	const SUB = ["", "*."];
	let i = 1;
	const domains = DOMAINNAME.reduce((acc, dn) => {
		const subDomains = SUB.map(s => s + (i <= 2 ? (name + ".") : i <= 3 ? (shortname + ".") : "") + dn);
		i++;
		return acc.concat(subDomains);
	}, [] as string[]);
	const domainsNb = domains.length;
	this.consolelog(2, "ACME: Domains:" + domains.join(" "));
	const csrDer = await CSR.csr({ jwk:serverKey, domains, encoding:"der" });
	const csr = PEM.packBlock({ type:"CERTIFICATE REQUEST", bytes:csrDer });
	const lines = [];
	let domainIter = 0;
	const challenges = {
		"dns-01": {
			init: async (args) => {
				return null;
			},
			zones: async ({ challenge }) => {
				this.consolelog(2, "ACME: Zones", DOMAINNAME);
				return DOMAINNAME;
			},
			set: async ({ challenge }) => {
				const domain = (challenge?.wildcard ? "*." : "") + challenge.identifier.value;
				this.consolelog(2, "ACME: Set " + domain);
				//for (let i = 0; i < challenge.challenges.length; i++)
					//if (challenge.challenges[i]["type"] == "dns-01") {
						//this.consolelog(2, "ACME: Set" + (challenge?.wildcard ? " (.*)" : ""), challenge);
					//}
				const line = "_acme-challenge." + challenge.identifier.value + ". 1 IN TXT " + challenge.keyAuthorizationDigest + "\n";
				this.consolelog(2, line);
				lines.push(line);
				if (domains.length == lines.length) {
					const dataA = { lines, action:"add" };
					const retA = await this.httpClient.post(this.SERVERURL + "/master/certificates.json", dataA).toPromise();
					this.consolelog(2, retA);
					//alert("AuthorizationDigests sent to DNS server");
				}
			},
			get: async (args) => {
				//this.consolelog(2, "ACME: Get: ", args);
			},
			remove: async ({ challenge }) => {
				const domain = (challenge?.wildcard ? "*." : "") + challenge.identifier.value;
				this.consolelog(2, "ACME: Remove " + domain);
				//for (let i = 0; i < challenge.challenges.length; i++)
					//if (challenge.challenges[i]["type"] == "dns-01") {
						//this.consolelog(2, "ACME: Remove" + (challenge?.wildcard ? " (.*)" : ""), challenge);
					//}
			},
			propagationDelay: 2000
		}
	};
	try {
		const pems = await acme.certificates.create({ account, accountKey, csr, domains, challenges });
		ret.fullChain = pems.cert + "\n" + pems.chain + "\n";
		this.consolelog(2, "##################################");
		this.consolelog(2, "ACME: Expiration " + pems.expires);
		this.consolelog(2, ret);
		this.consolelog(2, "##################################");
	} catch(e) {}
	const dataD = { lines, action:"delete" };
	const retD = await this.httpClient.post(this.SERVERURL + "/master/certificates.json", dataD).toPromise();
	this.consolelog(2, retD);
	return ret;
}

canActivate(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
	this.activateUrl = state.url;
	return this.splashDone ? true : this.router.navigate(["/splash"]);
}

}
