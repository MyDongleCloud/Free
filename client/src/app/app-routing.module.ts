import { NgModule } from '@angular/core';
import { PreloadAllModules, RouterModule, Routes } from '@angular/router';
import { Global } from './env';

const routes: Routes = [
	{
		path: 'about',
		canActivate: [Global],
		loadChildren: () => import('./about/about.module').then( m => m.AboutModule)
	},
	{
		path: 'dongle',
		canActivate: [Global],
		loadChildren: () => import('./dongle/dongle.module').then( m => m.DongleModule)
	},
	{
		path: 'help',
		canActivate: [Global],
		loadChildren: () => import('./help/help.module').then( m => m.HelpModule)
	},
	{
		path: '',
		canActivate: [Global],
		loadChildren: () => import('./home/home.module').then( m => m.HomeModule)
	},
	{
		path: 'login',
		loadChildren: () => import('./login/login.module').then( m => m.LoginModule)
	},
	{
		path: 'setup',
		canActivate: [Global],
		loadChildren: () => import('./setup/setup.module').then( m => m.SetupModule)
	},
	{
		path: 'splash',
		loadChildren: () => import('./splash/splash.module').then( m => m.SplashModule)
	},
	{
		path: '**',
		redirectTo: 'splash',
		pathMatch: 'full'
	},
];

@NgModule({
	imports: [
		RouterModule.forRoot(routes, { preloadingStrategy: PreloadAllModules })
	],
	exports: [RouterModule]
})

export class AppRoutingModule {

constructor() {}

}
