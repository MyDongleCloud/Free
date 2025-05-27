import { NgModule } from '@angular/core';
import { PreloadAllModules, RouterModule, Routes } from '@angular/router';
import { Global } from './env';

const routes: Routes = [
	{
		path: 'about',
		loadChildren: () => import('./about/about.module').then( m => m.AboutModule)
	},
	{
		path: 'dongle',
		loadChildren: () => import('./dongle/dongle.module').then( m => m.DongleModule)
	},
	{
		path: 'help',
		loadChildren: () => import('./help/help.module').then( m => m.HelpModule)
	},
	{
		path: 'selection',
		loadChildren: () => import('./selection/selection.module').then( m => m.SelectionModule)
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

constructor(private global: Global) {
	console.log("Version: " + global.VERSION);
	console.log("Platform: " + global.plt.platforms());
	global.currentUrl = "selection";
}

}
