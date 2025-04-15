import { display_comp_search_list } from "./list.js";
import { getData } from "./backend.js";
import { nav_next, nav_prev } from "./nav.js";
import { focus_next_el, focus_prev_el, find_match } from "./list.js";
import {
  show_selected,
  overlay_go_up,
  overlay_go_down,
  overlay_start_edit,
  overlay_exit_edit_mode,
  overlay_go_left,
  overlay_go_right,
  overlay_exit_table_mode,
  overlay_start_input,
  overlay_input,
  overlay_accept_input,
} from "./overlay.js";
import { input, overlay, main } from "../index.js";

export async function setup_key_listener() {
  document.addEventListener("keydown", async (key) => {
    if (document.activeElement == input) {
      handle_focused_main_input(key);
      return void "";
    }
    if (document.activeElement == overlay_input) {
      handle_focused_overlay_input(key);
      return void "";
    }
    if (!overlay.classList.contains("hidden")) {
      handle_focused_overlay_key(key);
      return void "";
    }
    await handle_key(key);
  });
}

function handle_focused_overlay_input(key) {
  switch (key.key) {
    // case "ArrowDown":
    //   focus_next_el();
    //   break;
    // case "ArrowUp":
    //   focus_prev_el();
    //   break;
    case "Enter":
      overlay_accept_input();
      document.activeElement.blur();
      break;
    case "Escape":
      document.activeElement.blur();
      break;
    default:
      break;
  }
}

function handle_focused_main_input(key) {
  switch (key.key) {
    case "ArrowDown":
      focus_next_el();
      break;
    case "ArrowUp":
      focus_prev_el();
      break;
    case "Enter":
      document.activeElement.blur();
      show_selected();
      break;
    case "Escape":
      document.activeElement.blur();
      overlay.classList.add("hidden");
      break;
    default:
      break;
  }
}

function handle_focused_overlay_key(key) {
  switch (key.key) {
    case "j":
      overlay_go_down();
      break;
    case "k":
      overlay_go_up();
      break;
    case "h":
      overlay_go_left();
      break;
    case "l":
      overlay_go_right();
      break;
    case "e":
      overlay_start_edit();
      break;
    case "i":
      key.preventDefault();
      overlay_start_input();
      break;
    case "x":
      overlay_exit_edit_mode();
      break;
    case "<":
      overlay_exit_table_mode();
      break;
    case "Escape":
      overlay_exit_edit_mode();
      overlay_exit_edit_mode();
      input.focus();
      overlay.classList.add("hidden");
      break;
    default:
      break;
  }
}

async function handle_key(key) {
  switch (key.key) {
    case "Escape":
      overlay.classList.add("hidden");
      break;
    case ">":
      console.log("nav_next");
      nav_next();
      break;
    case "<":
      console.log("nav_prev");
      nav_prev();
      break;
    case "i":
      key.preventDefault();
      input.focus();
      display_comp_search_list();
      find_match();
      break;
    case "J":
      display_comp_search_list();
      break;
    case "f":
      data = await getData("http://localhost:8080/data");
      console.log(data);
      main.innerText = JSON.stringify(data, null, 2);
      break;
    case "l":
      data = await getData("http://localhost:8080/data");
      if (!data) {
        document.getElementsByTagName("main")[0].innerText = "404";
        data = "404";
      } else {
        document.getElementsByTagName("main")[0].innerText = JSON.stringify(
          data,
          null,
          2,
        );
      }
      break;

    default:
      console.debug("pressed_key:", key.key);
      break;
  }
}
