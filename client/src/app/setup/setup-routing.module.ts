import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Setup } from './setup.page';

const routes: Routes = [{
	path: '',
	component: Setup
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class SetupRoutingModule {}
