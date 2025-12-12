import { NgModule } from '@angular/core';
import { LongPressDirective } from './longPress/longPress.directive';
import { IonicModule } from '@ionic/angular';
import { SidebarComponent } from './sidebar/sidebar.component';
import { TopbarComponent } from './topbar/topbar.component';
import { ToastComponent } from './toast/toast.component';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

@NgModule({
	declarations: [LongPressDirective, SidebarComponent, TopbarComponent, ToastComponent],
	exports: [LongPressDirective, SidebarComponent, TopbarComponent, ToastComponent],
	imports: [IonicModule, CommonModule, FormsModule]
})
export class ComponentsModule {}
