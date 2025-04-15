import { get_data } from "./script/backend.js";
import { setup_key_listener } from "./script/keys.js";
import { display_comp_search_list, find_match } from "./script/list.js";

export const main = document.getElementsByTagName("main")[0];
export const input = document.getElementById("main-input");
export const comp_list = document.createElement("ul");
export const overlay = document.createElement("overlay");

overlay.id = "overlay";
overlay.classList.add("hidden");
comp_list.dataset.current_el = 0;

main.appendChild(overlay);
main.appendChild(comp_list);

input.addEventListener("input", () => {
  find_match();
});

get_data()
setup_key_listener()

setTimeout(() => {
  display_comp_search_list();
  find_match();

  document.dispatchEvent(new KeyboardEvent('keydown', {key: 'i'}));

  setTimeout(() => {
    document.dispatchEvent(new KeyboardEvent('keydown', {key: 'Enter'}));
  }, 400);
}, 100);
