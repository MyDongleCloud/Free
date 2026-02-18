import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Delete } from './delete.page';

const routes: Routes = [{
	path: '',
	component: Delete
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class DeleteRoutingModule {}
