import { Injectable } from '@angular/core';

export interface Settings {
	language: string,
	powerUser: boolean,
	isDev: number,
	deviceId: string,
	email: string,
	welcomeSeen: boolean,
	reviewRequestLastTime,
	dontShowAgain: object,
	currentUrl: string,
}

export interface OnlineRet {
	error: number;
	reason: string;
}
