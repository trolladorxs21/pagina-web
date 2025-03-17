document.getElementById("select-folder").addEventListener("click", async () => {
    try {
        const dirHandle = await window.showDirectoryPicker();
        const fileList = document.getElementById("file-list");
        fileList.innerHTML = ""; // Limpiar lista antes de agregar nuevos archivos

        for await (const entry of dirHandle.values()) {
            if (entry.kind === "file") {
                const listItem = document.createElement("li");
                listItem.textContent = entry.name;
                fileList.appendChild(listItem);
            }
        }
    } catch (error) {
        console.error("Error al acceder a la carpeta:", error);
    }
});
