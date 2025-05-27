import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Selection } from './selection.page';

const routes: Routes = [{
	path: '',
	component: Selection
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class SelectionRoutingModule {}
