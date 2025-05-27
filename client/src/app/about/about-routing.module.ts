import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { About } from './about.page';

const routes: Routes = [{
	path: '',
	component: About
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class AboutRoutingModule {}
