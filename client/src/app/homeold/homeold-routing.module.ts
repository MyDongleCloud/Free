import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { HomeOld } from './homeold.page';

const routes: Routes = [{
	path: '',
	component: HomeOld
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class HomeOldRoutingModule {}
