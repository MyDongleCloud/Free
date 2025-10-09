import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { Backup } from './backup.page';

const routes: Routes = [{
	path: '',
	component: Backup,
}];

@NgModule({
	imports: [RouterModule.forChild(routes)],
	exports: [RouterModule]
})

export class BackupRoutingModule {}
