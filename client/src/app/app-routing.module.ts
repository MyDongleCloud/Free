import { NgModule } from '@angular/core';
import { PreloadAllModules, RouterModule, Routes } from '@angular/router';
import { Global } from './env';

const routes: Routes = [
	{
		path: 'about',
		loadChildren: () => import('./about/about.module').then( m => m.AboutModule)
	},
	{
		path: 'backup',
		canActivate: [Global],
		loadChildren: () => import('./backup/backup.module').then( m => m.BackupModule)
	},
	{
		path: 'dongle',
		canActivate: [Global],
		loadChildren: () => import('./dongle/dongle.module').then( m => m.DongleModule)
	},
	{
		path: 'find',
		loadChildren: () => import('./find/find.module').then( m => m.FindModule)
	},
	{
		path: 'help',
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
		path: 'permissions',
		canActivate: [Global],
		loadChildren: () => import('./permissions/permissions.module').then( m => m.PermissionsModule)
	},
	{
		path: 'profile',
		canActivate: [Global],
		loadChildren: () => import('./profile/profile.module').then( m => m.ProfileModule)
	},
	{
		path: 'serverlog',
		canActivate: [Global],
		loadChildren: () => import('./serverlog/serverlog.module').then( m => m.ServerLogModule)
	},
	{
		path: 'settings',
		canActivate: [Global],
		loadChildren: () => import('./settings/settings.module').then( m => m.SettingsModule)
	},
	{
		path: 'setup',
		loadChildren: () => import('./setup/setup.module').then( m => m.SetupModule)
	},
	{
		path: 'splash',
		loadChildren: () => import('./splash/splash.module').then( m => m.SplashModule)
	},
	{
		path: 'wrapper',
		canActivate: [Global],
		loadChildren: () => import('./wrapper/wrapper.module').then( m => m.WrapperModule)
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
