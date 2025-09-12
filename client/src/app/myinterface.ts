import { Injectable } from '@angular/core';

export interface Settings {
	language: string,
	powerUser: boolean,
	isDev: number,
	deviceId: string,
	account: string,
	token: string,
	email: string,
	space: Object,
	welcomeSeen: boolean,
	reviewRequestLastTime,
	dontShowAgain: object,
	currentUrl: string,
}

export interface Config {
}

export interface OnlineRet {
	error: number;
	reason: string;
}
