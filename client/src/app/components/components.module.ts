import { NgModule } from '@angular/core';
import { LongPressDirective } from './longPress/longPress.directive';
import { IonicModule } from '@ionic/angular';
import { SidebarComponent } from './sidebar/sidebar.component';
import { TopbarComponent } from './topbar/topbar.component';

@NgModule({
	declarations: [LongPressDirective, SidebarComponent, TopbarComponent],
	exports: [LongPressDirective, SidebarComponent, TopbarComponent],
	imports: [IonicModule]
})
export class ComponentsModule {}
