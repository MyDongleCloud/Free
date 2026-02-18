import { NgModule } from '@angular/core';
import { PreloadAllModules, RouterModule, Routes } from '@angular/router';

const routes: Routes = [
	{
		path: 'delete',
		loadChildren: () => import('./delete/delete.module').then( m => m.DeleteModule)
	},
	{
		path: 'login',
		loadChildren: () => import('./login/login.module').then( m => m.LoginModule)
	},
	{
		path: 'setup',
		loadChildren: () => import('./setup/setup.module').then( m => m.SetupModule)
	},
	{
		path: '**',
		redirectTo: 'login',
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
