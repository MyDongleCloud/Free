import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Find } from './find.page';

const routes: Routes = [{
	path: '',
	component: Find
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class FindRoutingModule {}
