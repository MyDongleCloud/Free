import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ServerLog } from './serverlog.page';

const routes: Routes = [{
	path: '',
	component: ServerLog
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule],
})

export class ServerLogRoutingModule {}
