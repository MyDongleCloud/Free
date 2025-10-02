import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { Permissions } from './permissions.page';

const routes: Routes = [{
	path: '',
	component: Permissions
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class PermissionsRoutingModule {}
