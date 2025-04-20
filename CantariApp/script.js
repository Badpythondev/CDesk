function cleanText() {
    // Get the input text
    let inputText = document.getElementById('inputText').value;

    // Remove ., ; numbers but preserve newlines and spaces between words
    let cleanedText = inputText.replace(/([A-Z]\:|["'.,;\d]|\/\:|\:\/)/g, '').replace(/[^\S\r\n]+/g, ' ').replace(/(\n\ )+/g,'\n').trim();

    // Display the cleaned text
    document.getElementById('outputText').innerText = cleanedText;
}

function copyText() {
    let outputText = document.getElementById('outputText').innerText;
    const textarea = document.createElement('textarea');
    textarea.value = outputText;
    document.body.appendChild(textarea);
    textarea.select();
    try {
        navigator.clipboard.writeText(textarea.value);
        //document.execCommand('copy');
        alert('Textul a fost copiat!');
    } catch (err) {
        alert('Copierea textului a eșuat.');
    }
    document.body.removeChild(textarea);
}
