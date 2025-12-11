import { Injectable } from '@angular/core';

export interface Settings {
	lang: string,
	welcomeTourShown: boolean,
	powerUser: boolean,
	dontShowAgain: object,
	bookmarks: string[];
}
