void main_loop(state_t *s) {
  xcb_generic_event_t *event;
  while (event = xcb_wait_for_event(s->c)) {
    switch (XCB_EVENT_RESPONSE_TYPE(event)) {
    case XCB_MAP_REQUEST:
      map_request(s, event);
      break;
		case XCB_CONFIGURE_REQUEST:
			configure_request(s, event);
			break;
		case XCB_DESTROY_NOTIFY:
			destroy_notify(s, event);
			break;
    case XCB_KEY_PRESS:
      key_press(s, event);
      break;
    case XCB_BUTTON_PRESS:
      button_press(s, event);
      break;
    case XCB_BUTTON_RELEASE:
      button_release(s);
      break;
    case XCB_MOTION_NOTIFY:
      motion_notify(s, event);
      break;
    case XCB_ENTER_NOTIFY:
      enter_notify(s, event);
      break;
		}

    free(event);
  }
}

void map_request(state_t *s, xcb_generic_event_t *ev) {
  xcb_map_request_event_t *e = (xcb_map_request_event_t *)ev;

	client_create(s, e->window);
}

void configure_request(state_t *s, xcb_generic_event_t *ev) {
	xcb_configure_request_event_t *e = (xcb_configure_request_event_t *) ev;

	uint16_t value_mask = 0;
	uint32_t value_list[7];
	int i = 0;

	client_t *cl = client_from_wid(s, e->window);
	if (cl != NULL) {
		if (e->value_mask & XCB_CONFIG_WINDOW_X) cl->x = e->x;
		if (e->value_mask & XCB_CONFIG_WINDOW_Y) cl->y = e->y;
		if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH) cl->width = e->width;
		if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT) cl->height = e->height;
	}

	if (e->value_mask & XCB_CONFIG_WINDOW_X) {
		value_mask |= XCB_CONFIG_WINDOW_X;
		value_list[i++] = e->x;
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_Y) {
		value_mask |= XCB_CONFIG_WINDOW_Y;
		value_list[i++] = e->y;
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
		value_mask |= XCB_CONFIG_WINDOW_WIDTH;
		value_list[i++] = e->width;
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
		value_mask |= XCB_CONFIG_WINDOW_HEIGHT;
		value_list[i++] = e->height;
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH) {
		value_mask |= XCB_CONFIG_WINDOW_BORDER_WIDTH;
		value_list[i++] = e->border_width;
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_SIBLING) {
		value_mask |= XCB_CONFIG_WINDOW_SIBLING;
		value_list[i++] = e->sibling;
	}
	if (e->value_mask & XCB_CONFIG_WINDOW_STACK_MODE) {
		value_mask |= XCB_CONFIG_WINDOW_STACK_MODE;
		value_list[i++] = e->stack_mode;
	}

	xcb_configure_window(s->c, e->window, value_mask, value_list);
	if (s->focus) {
		xcb_set_input_focus(s->c, XCB_INPUT_FOCUS_POINTER_ROOT, s->focus->wid, XCB_CURRENT_TIME);
	} else {
		xcb_set_input_focus(s->c, XCB_INPUT_FOCUS_POINTER_ROOT, s->root, XCB_CURRENT_TIME);
	}
	xcb_flush(s->c);
}

void destroy_notify(state_t *s, xcb_generic_event_t *ev) {
  xcb_destroy_notify_event_t *e = (xcb_destroy_notify_event_t *)ev;

	client_t *cl = client_from_wid(s, e->window);
	
	if (!cl) {
		return;
	}

	client_remove(s, cl);
}

void key_press(state_t *s, xcb_generic_event_t *ev) {
  xcb_key_press_event_t *e = (xcb_key_press_event_t *)ev;

  int length = sizeof(keybinds) / sizeof(keybinds[0]);
  for (int i = 0; i < length; i++) {
    if (key_cmp(s, keybinds[i], e->detail, e->state)) {
      keybinds[i].callback(s, keybinds[i].command);
    }
  }
}

void button_press(state_t *s, xcb_generic_event_t *ev) {
  xcb_button_press_event_t *e = (xcb_button_press_event_t *)ev;

	if (s->focus->fullscreen) {
		return;
	}

  uint32_t value_list[] = {XCB_STACK_MODE_ABOVE};

  xcb_configure_window(s->c, s->focus->wid, XCB_CONFIG_WINDOW_STACK_MODE, value_list);

  s->m->pressed_button = e->detail;
  s->m->root_x = e->root_x;
  s->m->root_y = e->root_y;

  xcb_grab_pointer(s->c, 0, s->root,
                   XCB_EVENT_MASK_POINTER_MOTION |
                 	 XCB_EVENT_MASK_BUTTON_RELEASE,
                   XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, s->root, XCB_NONE,
                   XCB_CURRENT_TIME);

  xcb_flush(s->c);
}

void button_release(state_t *s) {
  s->m->resizingcorner = CORNER_NONE;

  s->m->pressed_button = 0;

  xcb_ungrab_pointer(s->c, XCB_CURRENT_TIME);

  xcb_flush(s->c);
}

void motion_notify(state_t *s, xcb_generic_event_t *ev) {
  xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *)ev;

  uint32_t current_time = e->time;
  if ((current_time - s->lastmotiontime) <= 10) {
    return;
  }

  if (s->m->pressed_button == 1) {
		int x = s->focus->x + (e->root_x - s->m->root_x);
		int y = s->focus->y + (e->root_y - s->m->root_y);
    client_move(s, s->focus, x, y);
  } else if (s->m->pressed_button == 3) {
		client_resize(s, s->focus, e);
  }

  xcb_flush(s->c);

  s->m->root_x = e->root_x;
  s->m->root_y = e->root_y;
	s->lastmotiontime = current_time;
}

void enter_notify(state_t *s, xcb_generic_event_t *ev) {
  xcb_enter_notify_event_t *e = (xcb_enter_notify_event_t *)ev;

	client_t *cl = client_from_wid(s, e->event);

	client_focus(s, cl);
}
